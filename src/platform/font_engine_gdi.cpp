// GDI font engine implementation (Windows only)
// This wraps the existing GDI calls used in RTS.cpp for the font engine interface.
// On Windows, the existing direct GDI calls continue to work, so this file
// serves as a reference implementation and is not actively used yet.

#ifdef _WIN32

#include "font_engine.h"
#include <windows.h>

class GDIFontInstance : public IFontInstance {
    HFONT m_hFont;
    HDC m_hDC;
    FontMetrics m_metrics;

public:
    GDIFontInstance(const LOGFONT& lf) : m_hFont(nullptr), m_hDC(nullptr) {
        m_hDC = CreateCompatibleDC(NULL);
        m_hFont = CreateFontIndirect(&lf);
        if (m_hFont && m_hDC) {
            HFONT hOld = (HFONT)SelectObject(m_hDC, m_hFont);
            TEXTMETRIC tm;
            GetTextMetrics(m_hDC, &tm);
            m_metrics.height = tm.tmHeight;
            m_metrics.ascent = tm.tmAscent;
            m_metrics.descent = tm.tmDescent;
            m_metrics.internalLeading = tm.tmInternalLeading;
            m_metrics.externalLeading = tm.tmExternalLeading;
            m_metrics.aveCharWidth = tm.tmAveCharWidth;
            m_metrics.maxCharWidth = tm.tmMaxCharWidth;
            m_metrics.weight = tm.tmWeight;
            SelectObject(m_hDC, hOld);
        }
    }

    ~GDIFontInstance() override {
        if (m_hFont) DeleteObject(m_hFont);
        if (m_hDC) DeleteDC(m_hDC);
    }

    FontMetrics GetMetrics() const override { return m_metrics; }

    bool GetTextExtent(const wchar_t* str, int len, SIZE* extent) override {
        HFONT hOld = (HFONT)SelectObject(m_hDC, m_hFont);
        BOOL ok = GetTextExtentPoint32W(m_hDC, str, len, extent);
        SelectObject(m_hDC, hOld);
        return ok != FALSE;
    }

    bool GetCharExtent(wchar_t ch, SIZE* extent) override {
        return GetTextExtent(&ch, 1, extent);
    }

    bool GetGlyphOutline(const wchar_t* str, int len,
                          std::vector<PathCommand>& path) override {
        HFONT hOld = (HFONT)SelectObject(m_hDC, m_hFont);
        SetBkMode(m_hDC, TRANSPARENT);
        SetTextColor(m_hDC, 0xFFFFFF);

        BeginPath(m_hDC);
        TextOutW(m_hDC, 0, 0, str, len);
        EndPath(m_hDC);

        // Extract path data
        int nPoints = GetPath(m_hDC, NULL, NULL, 0);
        if (nPoints > 0) {
            std::vector<POINT> points(nPoints);
            std::vector<BYTE> types(nPoints);
            GetPath(m_hDC, points.data(), types.data(), nPoints);

            for (int i = 0; i < nPoints; i++) {
                PathCommand cmd;
                cmd.pt = points[i];
                switch (types[i] & ~PT_CLOSEFIGURE) {
                    case PT_MOVETO: cmd.type = PathCommandType::MoveTo; break;
                    case PT_LINETO: cmd.type = PathCommandType::LineTo; break;
                    case PT_BEZIERTO: cmd.type = PathCommandType::CubicBezierTo; break;
                    default: cmd.type = PathCommandType::LineTo; break;
                }
                path.push_back(cmd);
                if (types[i] & PT_CLOSEFIGURE) {
                    PathCommand close;
                    close.type = PathCommandType::CloseFigure;
                    close.pt = {0, 0};
                    path.push_back(close);
                }
            }
        }

        SelectObject(m_hDC, hOld);
        return true;
    }
};

class GDIFontEngine : public IFontEngine {
public:
    std::unique_ptr<IFontInstance> CreateFont(const LOGFONT& lf) override {
        return std::make_unique<GDIFontInstance>(lf);
    }
};

std::unique_ptr<IFontEngine> CreatePlatformFontEngine() {
    return std::make_unique<GDIFontEngine>();
}

IFontEngine& IFontEngine::GetInstance() {
    static auto engine = CreatePlatformFontEngine();
    return *engine;
}

#endif // _WIN32
