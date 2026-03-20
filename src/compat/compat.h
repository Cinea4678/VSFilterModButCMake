#pragma once
// Master compatibility header for cross-platform VSFilterMod
//
// On Windows: this is a no-op, the original headers are used
// On non-Windows: provides replacements for Windows/MFC/ATL/DirectShow types

#ifdef _WIN32
    // Windows - nothing needed, use native headers
#else
    #include "compat_windows_types.h"
    #include "compat_mfc_types.h"
#endif
