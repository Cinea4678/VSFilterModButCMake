// FreeType + HarfBuzz font engine implementation
// Used on macOS/Linux as replacement for Windows GDI text rendering

#ifndef _WIN32

#include "font_engine.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_IDS_H

#include <hb.h>
#include <hb-ft.h>

#include <fontconfig/fontconfig.h>

#include <mutex>
#include <unordered_map>

// ============================================================
// FreeType font instance
// ============================================================

class FreeTypeFontInstance : public IFontInstance {
    FT_Face m_face;
    hb_font_t* m_hbFont;
    int m_pixelHeight;
    FontMetrics m_metrics;
    bool m_valid;

public:
    FreeTypeFontInstance(FT_Library ftLib, const LOGFONT& lf)
        : m_face(nullptr), m_hbFont(nullptr), m_pixelHeight(0), m_valid(false)
    {
        // Use fontconfig to find the font file
        std::string fontPath = FindFontFile(lf);
        if (fontPath.empty()) return;

        FT_Error err = FT_New_Face(ftLib, fontPath.c_str(), 0, &m_face);
        if (err) return;

        // Set pixel size
        m_pixelHeight = lf.lfHeight < 0 ? -lf.lfHeight : lf.lfHeight;
        if (m_pixelHeight == 0) m_pixelHeight = 16; // default
        FT_Set_Pixel_Sizes(m_face, 0, m_pixelHeight);

        // Create HarfBuzz font
        m_hbFont = hb_ft_font_create(m_face, nullptr);

        // Cache metrics
        m_metrics.height = (int)(m_face->size->metrics.height >> 6);
        m_metrics.ascent = (int)(m_face->size->metrics.ascender >> 6);
        m_metrics.descent = (int)(-m_face->size->metrics.descender >> 6);
        m_metrics.internalLeading = m_metrics.height - m_metrics.ascent - m_metrics.descent;
        m_metrics.externalLeading = 0;
        m_metrics.aveCharWidth = (int)(m_face->size->metrics.max_advance >> 6) / 2;
        m_metrics.maxCharWidth = (int)(m_face->size->metrics.max_advance >> 6);
        m_metrics.weight = lf.lfWeight;

        m_valid = true;
    }

    ~FreeTypeFontInstance() override {
        if (m_hbFont) hb_font_destroy(m_hbFont);
        if (m_face) FT_Done_Face(m_face);
    }

    FontMetrics GetMetrics() const override { return m_metrics; }

    bool GetTextExtent(const wchar_t* str, int len, SIZE* extent) override {
        if (!m_valid || !str || len <= 0) {
            extent->cx = 0;
            extent->cy = m_pixelHeight;
            return false;
        }

        // Use HarfBuzz for text shaping
        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf32(buf, (const uint32_t*)str, len, 0, len);
        hb_buffer_guess_segment_properties(buf);
        hb_shape(m_hbFont, buf, nullptr, 0);

        unsigned int glyphCount;
        hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buf, &glyphCount);

        int totalWidth = 0;
        for (unsigned int i = 0; i < glyphCount; i++) {
            totalWidth += positions[i].x_advance >> 6;
        }

        extent->cx = totalWidth;
        extent->cy = m_metrics.height;

        hb_buffer_destroy(buf);
        return true;
    }

    bool GetCharExtent(wchar_t ch, SIZE* extent) override {
        return GetTextExtent(&ch, 1, extent);
    }

    bool GetGlyphOutline(const wchar_t* str, int len,
                          std::vector<PathCommand>& path) override {
        if (!m_valid || !str || len <= 0) return false;

        // Use HarfBuzz for shaping
        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf32(buf, (const uint32_t*)str, len, 0, len);
        hb_buffer_guess_segment_properties(buf);
        hb_shape(m_hbFont, buf, nullptr, 0);

        unsigned int glyphCount;
        hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buf, &glyphCount);
        hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buf, &glyphCount);

        int penX = 0, penY = 0;

        for (unsigned int i = 0; i < glyphCount; i++) {
            FT_UInt glyphIndex = infos[i].codepoint;
            int xOffset = positions[i].x_offset >> 6;
            int yOffset = positions[i].y_offset >> 6;
            int xAdvance = positions[i].x_advance >> 6;

            FT_Error err = FT_Load_Glyph(m_face, glyphIndex, FT_LOAD_NO_BITMAP);
            if (err) {
                penX += xAdvance;
                continue;
            }

            FT_Outline& outline = m_face->glyph->outline;
            int startX = penX + xOffset;
            int startY = penY + yOffset;

            // Convert FreeType outline to path commands
            ExtractOutline(outline, startX, startY, path);

            penX += xAdvance;
        }

        hb_buffer_destroy(buf);
        return true;
    }

private:
    void ExtractOutline(const FT_Outline& outline, int offsetX, int offsetY,
                         std::vector<PathCommand>& path) {
        int contourStart = 0;
        for (int c = 0; c < outline.n_contours; c++) {
            int contourEnd = outline.contours[c];
            bool firstPoint = true;

            for (int p = contourStart; p <= contourEnd; p++) {
                FT_Vector& pt = outline.points[p];
                int x = (int)(pt.x >> 6) + offsetX;
                int y = -(int)(pt.y >> 6) + offsetY; // Flip Y (FreeType Y is up)
                char tag = outline.tags[p];

                PathCommand cmd;
                cmd.pt.x = x;
                cmd.pt.y = y;

                if (firstPoint) {
                    cmd.type = PathCommandType::MoveTo;
                    firstPoint = false;
                } else if (FT_CURVE_TAG(tag) == FT_CURVE_TAG_ON) {
                    cmd.type = PathCommandType::LineTo;
                } else if (FT_CURVE_TAG(tag) == FT_CURVE_TAG_CONIC) {
                    cmd.type = PathCommandType::QuadSplineTo;
                } else if (FT_CURVE_TAG(tag) == FT_CURVE_TAG_CUBIC) {
                    cmd.type = PathCommandType::CubicBezierTo;
                } else {
                    cmd.type = PathCommandType::LineTo;
                }

                path.push_back(cmd);
            }

            // Close contour
            PathCommand close;
            close.type = PathCommandType::CloseFigure;
            close.pt = {0, 0};
            path.push_back(close);

            contourStart = contourEnd + 1;
        }
    }

    static std::string FindFontFile(const LOGFONT& lf) {
        FcConfig* config = FcInitLoadConfigAndFonts();
        if (!config) return "";

        FcPattern* pattern = FcPatternCreate();

        // Convert face name to UTF-8
        std::string faceName;
        const wchar_t* name = lf.lfFaceName;
        while (*name) {
            if (*name < 128) faceName += (char)*name;
            else {
                // Simple UTF-8 encoding
                if (*name < 0x800) {
                    faceName += (char)(0xC0 | (*name >> 6));
                    faceName += (char)(0x80 | (*name & 0x3F));
                } else {
                    faceName += (char)(0xE0 | (*name >> 12));
                    faceName += (char)(0x80 | ((*name >> 6) & 0x3F));
                    faceName += (char)(0x80 | (*name & 0x3F));
                }
            }
            name++;
        }

        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)faceName.c_str());

        if (lf.lfWeight >= FW_BOLD)
            FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
        if (lf.lfItalic)
            FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);

        FcConfigSubstitute(config, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        FcResult result;
        FcPattern* match = FcFontMatch(config, pattern, &result);

        std::string path;
        if (match) {
            FcChar8* file = nullptr;
            if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
                path = (const char*)file;
            }
            FcPatternDestroy(match);
        }

        FcPatternDestroy(pattern);
        FcConfigDestroy(config);

        return path;
    }
};

// ============================================================
// FreeType font engine
// ============================================================

class FreeTypeFontEngine : public IFontEngine {
    FT_Library m_ftLib;
    std::mutex m_mutex;

public:
    FreeTypeFontEngine() : m_ftLib(nullptr) {
        FT_Init_FreeType(&m_ftLib);
    }

    ~FreeTypeFontEngine() override {
        if (m_ftLib) FT_Done_FreeType(m_ftLib);
    }

    std::unique_ptr<IFontInstance> CreateFont(const LOGFONT& lf) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return std::make_unique<FreeTypeFontInstance>(m_ftLib, lf);
    }
};

// Factory function
std::unique_ptr<IFontEngine> CreatePlatformFontEngine() {
    return std::make_unique<FreeTypeFontEngine>();
}

// Singleton
IFontEngine& IFontEngine::GetInstance() {
    static auto engine = CreatePlatformFontEngine();
    return *engine;
}

#endif // !_WIN32
