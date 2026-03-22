// FreeType + HarfBuzz font engine implementation
// Used on macOS/Linux as replacement for Windows GDI text rendering

#ifndef _WIN32

#include "font_engine.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H

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

        // Get OS/2 table for Windows GDI-compatible metrics.
        // Windows uses usWinAscent/usWinDescent (not hhea ascender/descender)
        // for cell height calculation and TEXTMETRIC values.
        TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(m_face, FT_SFNT_OS2);

        // Set pixel size to match Windows GDI behavior:
        // - Positive lfHeight = cell height (ascender + descender in pixels)
        // - Negative lfHeight = em height (character height in pixels)
        // FreeType's FT_Set_Pixel_Sizes sets the em square size, so for positive
        // lfHeight we must scale down to produce matching cell height.
        int requestedHeight = lf.lfHeight < 0 ? -lf.lfHeight : lf.lfHeight;
        if (requestedHeight == 0) requestedHeight = 16;

        if (lf.lfHeight > 0 && m_face->units_per_EM > 0) {
            int cellUnits = 0;
            if (os2 && (os2->usWinAscent + os2->usWinDescent) > 0) {
                cellUnits = os2->usWinAscent + os2->usWinDescent;
            } else {
                cellUnits = m_face->ascender - m_face->descender;
            }
            if (cellUnits > 0) {
                m_pixelHeight = (int)((long long)requestedHeight * m_face->units_per_EM / cellUnits);
            } else {
                m_pixelHeight = requestedHeight;
            }
        } else {
            m_pixelHeight = requestedHeight;
        }
        FT_Set_Pixel_Sizes(m_face, 0, m_pixelHeight);

        // Create HarfBuzz font
        m_hbFont = hb_ft_font_create(m_face, nullptr);

        // Cache metrics to match Windows GDI TEXTMETRIC values.
        if (os2 && (os2->usWinAscent + os2->usWinDescent) > 0) {
            int upem = m_face->units_per_EM;
            m_metrics.ascent = (int)((long long)os2->usWinAscent * m_pixelHeight / upem);
            m_metrics.descent = (int)((long long)os2->usWinDescent * m_pixelHeight / upem);
            m_metrics.height = m_metrics.ascent + m_metrics.descent;
        } else {
            m_metrics.height = (int)(m_face->size->metrics.height >> 6);
            m_metrics.ascent = (int)(m_face->size->metrics.ascender >> 6);
            m_metrics.descent = (int)(-m_face->size->metrics.descender >> 6);
        }
        m_metrics.internalLeading = m_metrics.height - m_pixelHeight;
        if (m_metrics.internalLeading < 0) m_metrics.internalLeading = 0;
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

        int penX = 0;
        // GDI TextOutW(hdc, 0, 0, ...) positions the cell top at y=0,
        // so the baseline is at y=tmAscent. FreeType outlines have the
        // baseline at y=0. Shift down by ascender to match GDI convention.
        int baselineY = m_metrics.ascent;

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
            int startY = baselineY - yOffset;

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
        // Use FreeType's outline decomposition for correct handling of
        // implicit on-curve points and proper curve conversion.
        struct DecomposeCtx {
            std::vector<PathCommand>* path;
            int offsetX, offsetY;
            bool hasStart;
            POINT startPt; // first point of current contour

            static POINT makePt(const FT_Vector* v, int ox, int oy) {
                POINT p;
                p.x = (int)(v->x >> 6) + ox;
                p.y = -(int)(v->y >> 6) + oy; // flip Y
                return p;
            }
        };

        DecomposeCtx ctx;
        ctx.path = &path;
        ctx.offsetX = offsetX;
        ctx.offsetY = offsetY;
        ctx.hasStart = false;
        ctx.startPt = {0, 0};

        FT_Outline_Funcs funcs = {};

        funcs.move_to = [](const FT_Vector* to, void* user) -> int {
            auto* c = (DecomposeCtx*)user;
            // Close previous contour if any
            if (c->hasStart) {
                PathCommand close;
                close.type = PathCommandType::CloseFigure;
                close.pt = {0, 0};
                c->path->push_back(close);
            }
            PathCommand cmd;
            cmd.type = PathCommandType::MoveTo;
            cmd.pt = DecomposeCtx::makePt(to, c->offsetX, c->offsetY);
            c->path->push_back(cmd);
            c->startPt = cmd.pt;
            c->hasStart = true;
            return 0;
        };

        funcs.line_to = [](const FT_Vector* to, void* user) -> int {
            auto* c = (DecomposeCtx*)user;
            PathCommand cmd;
            cmd.type = PathCommandType::LineTo;
            cmd.pt = DecomposeCtx::makePt(to, c->offsetX, c->offsetY);
            c->path->push_back(cmd);
            return 0;
        };

        // Quadratic bezier (conic) → convert to cubic bezier
        // Q(P0, P1, P2) → C(P0, P0+2/3*(P1-P0), P2+2/3*(P1-P2), P2)
        funcs.conic_to = [](const FT_Vector* ctrl, const FT_Vector* to, void* user) -> int {
            auto* c = (DecomposeCtx*)user;
            POINT ctrlPt = DecomposeCtx::makePt(ctrl, c->offsetX, c->offsetY);
            POINT toPt = DecomposeCtx::makePt(to, c->offsetX, c->offsetY);

            // Get the current point (last emitted point)
            POINT p0 = {0, 0};
            if (!c->path->empty()) {
                p0 = c->path->back().pt;
            }

            // Convert quadratic to cubic: C1 = P0 + 2/3*(P1-P0), C2 = P2 + 2/3*(P1-P2)
            PathCommand c1;
            c1.type = PathCommandType::CubicBezierTo;
            c1.pt.x = p0.x + (2 * (ctrlPt.x - p0.x) + 1) / 3;
            c1.pt.y = p0.y + (2 * (ctrlPt.y - p0.y) + 1) / 3;
            c->path->push_back(c1);

            PathCommand c2;
            c2.type = PathCommandType::CubicBezierTo;
            c2.pt.x = toPt.x + (2 * (ctrlPt.x - toPt.x) + 1) / 3;
            c2.pt.y = toPt.y + (2 * (ctrlPt.y - toPt.y) + 1) / 3;
            c->path->push_back(c2);

            PathCommand end;
            end.type = PathCommandType::CubicBezierTo;
            end.pt = toPt;
            c->path->push_back(end);

            return 0;
        };

        funcs.cubic_to = [](const FT_Vector* ctrl1, const FT_Vector* ctrl2,
                            const FT_Vector* to, void* user) -> int {
            auto* c = (DecomposeCtx*)user;

            PathCommand c1;
            c1.type = PathCommandType::CubicBezierTo;
            c1.pt = DecomposeCtx::makePt(ctrl1, c->offsetX, c->offsetY);
            c->path->push_back(c1);

            PathCommand c2;
            c2.type = PathCommandType::CubicBezierTo;
            c2.pt = DecomposeCtx::makePt(ctrl2, c->offsetX, c->offsetY);
            c->path->push_back(c2);

            PathCommand end;
            end.type = PathCommandType::CubicBezierTo;
            end.pt = DecomposeCtx::makePt(to, c->offsetX, c->offsetY);
            c->path->push_back(end);

            return 0;
        };

        FT_Outline outline_copy = outline;
        FT_Outline_Decompose(&outline_copy, &funcs, &ctx);

        // Close the last contour
        if (ctx.hasStart) {
            PathCommand close;
            close.type = PathCommandType::CloseFigure;
            close.pt = {0, 0};
            path.push_back(close);
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
