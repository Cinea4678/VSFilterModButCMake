#pragma once
// Cross-platform replacements for Windows basic types

#ifndef _WIN32

#include <cstdint>
#include <cstddef>
#include <climits>
#include <cwchar>

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
typedef float       FLOAT;

typedef int64_t     __int64;
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
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)

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

// Font weight constants
#define FW_DONTCARE     0
#define FW_THIN         100
#define FW_NORMAL       400
#define FW_BOLD         700
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

// min/max macros
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

// __stdcall, __cdecl - no-ops on non-Windows
#define __stdcall
#define __cdecl
#define __declspec(x)
#define CALLBACK
#define WINAPI

// MSVC-specific integer literal suffixes
// Note: code using 10000000i64 needs to be changed to 10000000LL

// memsetd - fill memory with DWORD values (from dsutil, needed cross-platform)
inline void memsetd(void* dst, unsigned int c, int nbytes) {
    for (int i = 0; i < nbytes / (int)sizeof(uint32_t); i++)
        ((uint32_t*)dst)[i] = c;
}

// __forceinline for non-MSVC compilers
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

#endif // !_WIN32
