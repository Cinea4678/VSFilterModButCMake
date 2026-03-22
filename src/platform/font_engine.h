#pragma once
// Abstract font engine interface for cross-platform text rendering.
//
// On Windows: implemented by GDI (font_engine_gdi.cpp)
// On macOS/Linux: implemented by FreeType + HarfBuzz (font_engine_freetype.cpp)

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include "../compat/compat_windows_types.h"
#endif

// Path command types (matching TrueType outline types)
enum class PathCommandType {
    MoveTo,
    LineTo,
    QuadSplineTo,  // TT_PRIM_QSPLINE
    CubicBezierTo, // TT_PRIM_CSPLINE
    CloseFigure,
};

struct PathCommand {
    PathCommandType type;
    POINT pt;  // coordinates in font units (scaled)
};

// Font metrics
struct FontMetrics {
    int height;
    int ascent;
    int descent;
    int internalLeading;
    int externalLeading;
    int aveCharWidth;
    int maxCharWidth;
    int weight;
};

// Abstract font handle
class IFontInstance {
public:
    virtual ~IFontInstance() {}

    // Get font metrics (equivalent to GetTextMetrics)
    virtual FontMetrics GetMetrics() const = 0;

    // Measure text width/height (equivalent to GetTextExtentPoint32)
    virtual bool GetTextExtent(const wchar_t* str, int len, SIZE* extent) = 0;

    // Get glyph outline as path commands (equivalent to BeginPath/TextOut/EndPath)
    virtual bool GetGlyphOutline(const wchar_t* str, int len,
                                  std::vector<PathCommand>& path) = 0;

    // Get single character extent
    virtual bool GetCharExtent(wchar_t ch, SIZE* extent) = 0;
};

// Abstract font engine (factory)
class IFontEngine {
public:
    virtual ~IFontEngine() {}

    // Create or retrieve a cached font from LOGFONT (equivalent to CreateFontIndirect)
    virtual std::shared_ptr<IFontInstance> CreateFont(const LOGFONT& lf) = 0;

    // Get singleton instance
    static IFontEngine& GetInstance();
};

// Platform-specific factory
std::unique_ptr<IFontEngine> CreatePlatformFontEngine();
