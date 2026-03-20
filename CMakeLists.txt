cmake_minimum_required(VERSION 3.16)
project(VSFilterMod LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/compat
)

# Platform-specific dependencies
if(NOT WIN32)
    find_package(Freetype REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
endif()

# Build libraries
add_subdirectory(src/zlib)
add_subdirectory(src/libpng)
add_subdirectory(src/subtitles)
add_subdirectory(src/subpic)
add_subdirectory(src/platform)

if(VSFILTERMOD_BUILD_DIRECTSHOW)
    add_subdirectory(src/BaseClasses)
    add_subdirectory(src/dsutil)
    add_subdirectory(src/basevideofilter)
    add_subdirectory(src/vsfilter)
endif()
