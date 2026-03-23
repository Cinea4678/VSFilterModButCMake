#pragma once
// Cross-platform replacements for Windows basic types

#ifndef _WIN32

// Force UNICODE mode to match CString = CStringW in compat_mfc_types.h
#ifndef UNICODE
#define UNICODE
#endif

#include <cstdint>
#include <cstddef>
#include <climits>
#include <cwchar>
#include <cstring>

// Basic Windows types
typedef uint8_t     BYTE;
typedef uint16_t    WORD;
typedef uint32_t    DWORD;
typedef int32_t     LONG;
typedef uint32_t    ULONG;
typedef int64_t     LONGLONG;
typedef uint64_t    ULONGLONG;
typedef int16_t     SHORT;
typedef uint16_t    USHORT;
typedef int32_t     INT;
typedef uint32_t    UINT;
typedef int         BOOL;
typedef unsigned char UCHAR;
// Note: 'byte' conflicts with std::byte in C++17 when 'using namespace std' is present.
// We don't define it globally. Files using 'byte' should use BYTE or unsigned char instead.
// For source files that need 'byte', define it locally after their 'using namespace std'.
typedef float       FLOAT;

// __int64: use long long so "unsigned __int64" works
#define __int64 long long
typedef uintptr_t   DWORD_PTR;
typedef uintptr_t   UINT_PTR;
typedef intptr_t    INT_PTR;
typedef intptr_t    LONG_PTR;
typedef size_t      SIZE_T;

typedef void*       LPVOID;
typedef const void* LPCVOID;
typedef char*       LPSTR;
typedef const char* LPCSTR;
typedef wchar_t*    LPWSTR;
typedef const wchar_t* LPCWSTR;

#ifdef UNICODE
typedef wchar_t     TCHAR;
typedef LPWSTR      LPTSTR;
typedef LPCWSTR     LPCTSTR;
#define _T(x)       L##x
#define _tcslen     wcslen
#define _tcscpy     wcscpy
#define _tcscat     wcscat
#define _tcscmp     wcscmp
#define _tcsicmp    wcscasecmp
#define _tcsncmp    wcsncmp
#define _stprintf   swprintf
#define _stscanf    swscanf
#else
typedef char        TCHAR;
typedef LPSTR       LPTSTR;
typedef LPCSTR      LPCTSTR;
#define _T(x)       x
#define _tcslen     strlen
#define _tcscpy     strcpy
#define _tcscat     strcat
#define _tcscmp     strcmp
#define _tcsicmp    strcasecmp
#define _tcsncmp    strncmp
#define _stprintf   sprintf
#define _stscanf    sscanf
#endif

// Boolean constants
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((void*)(intptr_t)-1)
#endif

// Handle types (opaque pointers)
typedef void*       HANDLE;
typedef void*       HWND;
typedef void*       HDC;
typedef void*       HFONT;
typedef void*       HBITMAP;
typedef void*       HINSTANCE;
typedef void*       HMODULE;
typedef void*       HRGN;
typedef void*       HBRUSH;
typedef void*       HPEN;
typedef void*       HGDIOBJ;

// COM-like types
typedef int32_t     HRESULT;
#define S_OK        ((HRESULT)0)
#define S_FALSE     ((HRESULT)1)
#define E_FAIL      ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG  ((HRESULT)0x80070057L)
#define E_NOTIMPL     ((HRESULT)0x80004001L)
#define E_UNEXPECTED  ((HRESULT)0x8000FFFFL)
#define E_POINTER     ((HRESULT)0x80004003L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_ABORT       ((HRESULT)0x80004004L)
#define NOERROR       0
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)

// DirectShow-like utility macros
#define NAME(x) (x)
#define CheckPointer(p, ret) do { if ((p) == nullptr) return (ret); } while(0)

// QI macro (DirectShow QueryInterface helper) - simplified, always falls through
#define QI(i)

// MulDiv replacement
inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
    if (nDenominator == 0) return -1;
    return (int)(((int64_t)nNumber * nNumerator) / nDenominator);
}

// GDI types
typedef uint32_t    COLORREF;
#define RGB(r,g,b)  ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb)>>16))

// GUID/IID
struct GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
};
typedef GUID IID;
typedef GUID CLSID;
typedef const GUID& REFGUID;
typedef const IID&  REFIID;
typedef const CLSID& REFCLSID;

// DirectShow REFERENCE_TIME
typedef int64_t REFERENCE_TIME;

// LOGFONT structure (needed for font handling)
#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

typedef struct tagLOGFONTW {
    LONG    lfHeight;
    LONG    lfWidth;
    LONG    lfEscapement;
    LONG    lfOrientation;
    LONG    lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    wchar_t lfFaceName[LF_FACESIZE];
} LOGFONTW, *LPLOGFONTW;

typedef LOGFONTW LOGFONT;
typedef LOGFONTW* LPLOGFONT;

typedef struct tagLOGFONTA {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[32];
} LOGFONTA, *LPLOGFONTA;

// Font weight constants
#define FW_DONTCARE     0
#define FW_THIN         100
#define FW_NORMAL       400
#define FW_BOLD         700
#define FW_LIGHT        300
#define FW_SEMIBOLD     600
#define FW_BLACK        900

// Charset constants
#define DEFAULT_CHARSET     1
#define ANSI_CHARSET        0
#define SYMBOL_CHARSET      2
#define SHIFTJIS_CHARSET    128
#define HANGUL_CHARSET      129
#define GB2312_CHARSET      134
#define CHINESEBIG5_CHARSET 136
#define OEM_CHARSET         255
#define HANGEUL_CHARSET     129
#define JOHAB_CHARSET       130
#define HEBREW_CHARSET      177
#define ARABIC_CHARSET      178
#define GREEK_CHARSET       161
#define TURKISH_CHARSET     162
#define VIETNAMESE_CHARSET  163
#define THAI_CHARSET        222
#define EASTEUROPE_CHARSET  238
#define RUSSIAN_CHARSET     204
#define MAC_CHARSET         77
#define BALTIC_CHARSET      186

// Font output precision
#define OUT_TT_PRECIS       4
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define DEFAULT_PITCH       0
#define FF_DONTCARE         0

// GDI map modes
#define MM_TEXT     1

// GDI background modes
#define TRANSPARENT 1
#define GM_ADVANCED 2

// GDI function stubs
inline int SetGraphicsMode(HDC, int) { return 0; }
inline BOOL SetWorldTransform(HDC, const void*) { return FALSE; }
inline BOOL ModifyWorldTransform(HDC, const void*, DWORD) { return FALSE; }

// COM stubs
inline void* CoTaskMemAlloc(size_t) { return nullptr; }
inline void CoTaskMemFree(void*) {}
#define USES_CONVERSION
#define W2A(x) ""

// LoadLibrary/GetProcAddress stubs
#define LoadLibrary(x) ((HMODULE)nullptr)
inline void* GetProcAddress(HMODULE, const char*) { return nullptr; }
inline BOOL FreeLibrary(HMODULE) { return FALSE; }
#define IN
#define PVOID void*

// _tcscpy_s replacement
#ifndef _tcscpy_s
#define _tcscpy_s(dst, src) wcsncpy(dst, (const wchar_t*)(src), sizeof(dst)/sizeof(dst[0]))
#endif

// GDI handle types
typedef void* HDC;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HBRUSH;
typedef void* HPEN;
typedef void* HGDIOBJ;

// Misc Windows constants
#define MAX_PATH 260

// POINT, SIZE, RECT (basic versions)
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT;

typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE;

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

// min/max are provided via std::min/std::max from <algorithm>
// Do NOT define them as macros - that breaks C++ standard library headers.

// __stdcall, __cdecl - no-ops on non-Windows
#define __stdcall
#define __cdecl
#define __declspec(x)
#define CALLBACK
#define WINAPI

// MSVC CRT function replacements
inline int _wtoi(const wchar_t* s) { return (int)wcstol(s, nullptr, 10); }
inline double _wtof(const wchar_t* s) { return wcstof(s, nullptr); }
#define _stprintf swprintf
#define _vstprintf_s(buf, len, fmt, args) vswprintf(buf, len, fmt, args)
#define _vsctprintf(fmt, args) vswprintf(nullptr, 0, fmt, args)
#define _vscwprintf(fmt, args) vswprintf(nullptr, 0, fmt, args)
#define vswprintf_s(buf, len, fmt, args) vswprintf(buf, len, fmt, args)
#define _tfopen_s(pf, fn, mode) (*(pf) = fopen(fn, mode), *(pf) ? 0 : -1)

// Character encoding stubs
typedef struct { UINT ciCharset; UINT ciACP; } CHARSETINFO;
#define TCI_SRCCHARSET 1
#define CP_ACP 0
inline BOOL TranslateCharsetInfo(DWORD* src, CHARSETINFO* cs, DWORD flags) {
    if (cs) { cs->ciCharset = 0; cs->ciACP = 65001; } // Default to UTF-8
    return TRUE;
}
inline BOOL IsDBCSLeadByteEx(UINT, BYTE) { return FALSE; }
inline int WideCharToMultiByte(UINT, DWORD, const wchar_t* src, int srcLen, char* dst, int dstLen, const char*, BOOL*) {
    if (!src) return 0;
    if (srcLen < 0) srcLen = (int)wcslen(src) + 1;
    // Simple wchar→char narrowing (ASCII subset)
    if (!dst || dstLen == 0) return srcLen;
    for (int i = 0; i < srcLen && i < dstLen; i++) dst[i] = (char)src[i];
    return srcLen < dstLen ? srcLen : dstLen;
}
inline int MultiByteToWideChar(UINT, DWORD, const char* src, int srcLen, wchar_t* dst, int dstLen) {
    if (!src) return 0;
    if (srcLen < 0) srcLen = (int)strlen(src) + 1;
    if (!dst || dstLen == 0) return srcLen;
    for (int i = 0; i < srcLen && i < dstLen; i++) dst[i] = (wchar_t)(unsigned char)src[i];
    return srcLen < dstLen ? srcLen : dstLen;
}
#define _istdigit iswdigit
#define _istupper iswupper
#define _tcsnicmp wcsncasecmp
#define wcsnicmp wcsncasecmp
#define MAXLONG 0x7fffffff
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef TEXT
#define TEXT(x) L##x
#endif
inline int AddFontResource(const wchar_t*) { return 0; }
inline DWORD GetModuleFileName(HMODULE, wchar_t*, DWORD) { return 0; }
inline DWORD GetTempPath(DWORD, wchar_t*) { return 0; }

// GDI DPI stubs
#define LOGPIXELSY 90
inline HDC GetDC(void*) { return nullptr; }
inline int GetDeviceCaps(HDC, int) { return 96; } // Default 96 DPI
inline void ReleaseDC(void*, HDC) {}

#define _TRUNCATE ((size_t)-1)
#define CP_THREAD_ACP 3

// ATL string conversion macros (no-ops in UNICODE mode)
template<int t_nBufferLength = 128>
class CT2WEX {
    const wchar_t* m_str;
public:
    CT2WEX(const wchar_t* s, UINT = CP_ACP) : m_str(s) {}
    operator const wchar_t*() const { return m_str; }
};

// Secure CRT function replacements
#ifndef strncpy_s
inline int strncpy_s(char* dst, size_t dstSz, const char* src, size_t count) {
    if (!dst || !dstSz) return -1;
    size_t n = (count < dstSz - 1) ? count : dstSz - 1;
    if (src) strncpy(dst, src, n);
    dst[n] = 0;
    return 0;
}
#endif
#ifndef wcsncpy_s
inline int wcsncpy_s(wchar_t* dst, size_t dstSz, const wchar_t* src, size_t count) {
    if (!dst || !dstSz) return -1;
    size_t n = (count < dstSz - 1) ? count : dstSz - 1;
    if (src) wcsncpy(dst, src, n);
    dst[n] = 0;
    return 0;
}
#endif
inline DWORD GetTempFileName(const wchar_t*, const wchar_t*, UINT, wchar_t*) { return 0; }

// MessageBox stubs
#define MB_OK           0
#define MB_OKCANCEL     1
#define MB_ICONWARNING  0x30
#define MB_ICONERROR    0x10
#define IDOK            1
#define IDCANCEL        2
inline int AfxMessageBox(const wchar_t*, UINT = MB_OK, UINT = 0) { return IDOK; }

// Integer limits
#ifndef _I64_MAX
#define _I64_MAX 0x7fffffffffffffffLL
#endif

// TCHAR function mappings (UNICODE mode)
#define _tcschr wcschr
#define _tcsrchr wcsrchr
#define _tcstol wcstol
#define _tfopen _wfopen
#define _tremove _wremove
// Wide-char file operations (stubs using narrow conversion)
#include <cstdio>
inline FILE* _wfopen(const wchar_t* fn, const wchar_t* mode) {
    char fn8[260], mode8[16];
    for (int i = 0; fn[i] && i < 259; i++) { fn8[i] = (char)fn[i]; fn8[i+1] = 0; }
    for (int i = 0; mode[i] && i < 15; i++) { mode8[i] = (char)mode[i]; mode8[i+1] = 0; }
    return fopen(fn8, mode8);
}
inline int _wremove(const wchar_t* fn) {
    char fn8[260];
    for (int i = 0; fn[i] && i < 259; i++) { fn8[i] = (char)fn[i]; fn8[i+1] = 0; }
    return remove(fn8);
}
#define _istspace iswspace

// Unused parameter macro
#define UNREFERENCED_PARAMETER(p) (void)(p)

// MSVC-specific integer literal suffixes
// Note: code using 10000000i64 needs to be changed to 10000000LL

// memsetd - fill memory with DWORD values (from dsutil, needed cross-platform)
inline void memsetd(void* dst, unsigned int c, int nbytes) {
    for (int i = 0; i < nbytes / (int)sizeof(uint32_t); i++)
        ((uint32_t*)dst)[i] = c;
}

// Debug macros
#include <cassert>
#define ASSERT(x) assert(x)
#define VERIFY(x) ((void)(x))
#define EXECUTE_ASSERT(x) ASSERT(x)
#define TRACE(...) ((void)0)
#define DebugBreak() ((void)0)

// countof macro (array element count)
#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// CHAR type
typedef char CHAR;

// __forceinline for non-MSVC compilers
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

// MSVC aligned memory allocation replacements
#include <cstdlib>
#include <cstring>
inline void* _aligned_malloc(size_t size, size_t alignment) {
    void* ptr = nullptr;
    posix_memalign(&ptr, alignment, size);
    return ptr;
}
inline void* _aligned_realloc(void* ptr, size_t size, size_t alignment) {
    void* newptr = _aligned_malloc(size, alignment);
    if (newptr && ptr) {
        memcpy(newptr, ptr, size); // Note: copies up to new size (caller must handle)
        free(ptr);
    }
    return newptr;
}
inline void _aligned_free(void* ptr) {
    free(ptr);
}

// GDI function stubs (will be replaced by FreeType in Phase 3)
inline HDC CreateCompatibleDC(HDC) { return nullptr; }
inline void DeleteDC(HDC) {}
inline int SetBkMode(HDC, int) { return 0; }
inline COLORREF SetTextColor(HDC, COLORREF) { return 0; }
inline int SetMapMode(HDC, int) { return 0; }
inline HFONT SelectFont(HDC, HFONT) { return nullptr; }
inline HFONT CreateFontIndirect(const LOGFONT*) { return nullptr; }
inline void DeleteFont(HFONT) {}

inline BOOL GetTextExtentPoint32W(HDC, const wchar_t*, int, SIZE* s) {
    if (s) { s->cx = 0; s->cy = 0; }
    return FALSE;
}

inline DWORD GetGlyphOutline(HDC, UINT, UINT, void*, DWORD, void*, const void*) {
    return (DWORD)-1;
}

// GDI path functions (stubs)
inline BOOL BeginPath(HDC) { return FALSE; }
inline BOOL EndPath(HDC) { return FALSE; }
inline BOOL CloseFigure(HDC) { return FALSE; }
inline BOOL AbortPath(HDC) { return FALSE; }
inline int GetPath(HDC, POINT*, BYTE*, int) { return -1; }
inline BOOL TextOutW(HDC, int, int, const wchar_t*, int) { return FALSE; }
inline BOOL FlattenPath(HDC) { return FALSE; }

// GetTextMetrics stub (TEXTMETRICW* passed as void* since it's defined in compat_mfc_types.h)
inline BOOL GetTextMetricsW(HDC, void*) { return FALSE; }
#define GetTextMetrics GetTextMetricsW

// GetKerningPairs stub
typedef struct tagKERNINGPAIR {
    WORD wFirst;
    WORD wSecond;
    int iKernAmount;
} KERNINGPAIR;
inline DWORD GetKerningPairs(HDC, DWORD, KERNINGPAIR*) { return 0; }

// CFont stub class for non-Windows
class CFont {
    LOGFONT m_lf;
    HFONT m_hFont;
public:
    CFont() : m_lf{}, m_hFont(nullptr) {}
    BOOL CreateFontIndirect(const LOGFONT* lf) {
        if (lf) m_lf = *lf;
        m_hFont = ::CreateFontIndirect(lf);
        return TRUE;  // Stub: always succeeds
    }
    operator HFONT() const { return m_hFont; }
};

// GetVersion stub (returns 0x0600 = Vista)
inline DWORD GetVersion() { return 0x0600; }

// RGBQUAD - used extensively in VobSub image handling
typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

// Bitmap structures (used in VobSub BMP export)
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

// CMemFile - simple in-memory file buffer (stub for non-Windows)
#include <vector>
class CMemFile {
    std::vector<BYTE> m_data;
    size_t m_pos;
public:
    CMemFile() : m_pos(0) {}
    CMemFile(UINT /*nGrowBytes*/) : m_pos(0) {}
    CMemFile(BYTE* pBuffer, UINT nBufferSize, UINT /*nGrowBytes*/ = 0)
        : m_data(pBuffer, pBuffer + nBufferSize), m_pos(0) {}
    void Write(const void* data, size_t len) {
        const BYTE* p = (const BYTE*)data;
        if (m_pos + len > m_data.size()) m_data.resize(m_pos + len);
        memcpy(&m_data[m_pos], p, len);
        m_pos += len;
    }
    void SetLength(size_t len) { m_data.resize(len); m_pos = 0; }
    size_t GetLength() const { return m_data.size(); }
    size_t GetPosition() const { return m_pos; }
    BYTE* Detach() {
        BYTE* p = (BYTE*)malloc(m_data.size());
        if (p) memcpy(p, m_data.data(), m_data.size());
        m_data.clear(); m_pos = 0;
        return p;
    }
    void SeekToBegin() { m_pos = 0; }
    // Seek modes compatible with MFC: 0=begin, 1=current, 2=end
    size_t Seek(size_t offset, UINT from) {
        if (from == 0) m_pos = offset;
        else if (from == 1) m_pos += offset;
        else if (from == 2) m_pos = m_data.size() + offset;
        if (m_pos > m_data.size()) m_pos = m_data.size();
        return m_pos;
    }
    size_t Read(void* buf, size_t len) {
        size_t avail = m_data.size() - m_pos;
        size_t n = len < avail ? len : avail;
        memcpy(buf, &m_data[m_pos], n);
        m_pos += n;
        return n;
    }
    void Close() { m_data.clear(); m_pos = 0; }
};

#endif // !_WIN32
