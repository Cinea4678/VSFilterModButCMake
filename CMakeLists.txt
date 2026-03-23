cmake_minimum_required(VERSION 3.16)
project(VSFilterMod LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable PIC for all targets (static libs are linked into shared plugin)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Common options
option(VSFILTERMOD_BUILD_DIRECTSHOW "Build DirectShow filter (Windows only)" ON)

# On non-Windows, disable DirectShow components
if(NOT WIN32)
    set(VSFILTERMOD_BUILD_DIRECTSHOW OFF)
endif()

# Common compile definitions
add_compile_definitions(
    _VSMOD
    $<$<PLATFORM_ID:Windows>:WINVER=0x0600>
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0600>
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
    $<$<PLATFORM_ID:Windows>:UNICODE>
    $<$<PLATFORM_ID:Windows>:_UNICODE>
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG>
    $<$<CONFIG:MinSizeRel>:NDEBUG>
)

# Clang: enable __super support and other MSVC extensions used in the codebase
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fms-extensions)
endif()

# MSVC-specific flags
if(MSVC)
    add_compile_options(
        /MP             # Multi-processor compilation
        /fp:fast        # Fast floating point
        /Zc:throwingNew # Assume operator new throws
        /Zc:strictStrings-  # Allow deprecated string literal conversion
        $<$<CONFIG:Release>:/Oy>   # Omit frame pointers
        $<$<CONFIG:Release>:/GL>   # Whole program optimization
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>  # Link-time code generation
    )
endif()

# Include directories available to all targets
include_directories(
    ${CMAKE_SOURCE_DIR}/src/compat
)
# The include/ directory contains project-specific headers (moreuuids.h, Version.h, etc.)
if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/include)
endif()

# Platform-specific dependencies
if(NOT WIN32)
    find_package(Freetype REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
endif()

# On ARM64 (Apple Silicon), fetch sse2neon for SSE2→NEON translation
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    include(FetchContent)
    FetchContent_Declare(
        sse2neon
        GIT_REPOSITORY https://github.com/DLTcollab/sse2neon.git
        GIT_TAG        v1.7.0
    )
    FetchContent_MakeAvailable(sse2neon)
    # Make sse2neon.h available globally
    include_directories(${sse2neon_SOURCE_DIR})
endif()

# Enable MASM for 32-bit Windows builds (dsutil assembly files)
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    enable_language(ASM_MASM)
endif()

# Build third-party libraries
add_subdirectory(src/zlib)
add_subdirectory(src/libpng)

# Windows DirectShow support libraries
# Must be defined before subtitles/subpic/plugins which conditionally link them
if(VSFILTERMOD_BUILD_DIRECTSHOW)
    add_subdirectory(src/BaseClasses)
    add_subdirectory(src/dsutil)
    add_subdirectory(src/basevideofilter)
endif()

# Core libraries
add_subdirectory(src/subtitles)
add_subdirectory(src/subpic)
add_subdirectory(src/platform)

# Cross-platform VapourSynth/AviSynth plugin
add_subdirectory(src/plugins)

# Windows DirectShow filter
if(VSFILTERMOD_BUILD_DIRECTSHOW)
    add_subdirectory(src/vsfilter)
endif()
