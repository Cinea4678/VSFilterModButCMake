add_library(dsutil STATIC
    DSMPropertyBag.cpp
    DSUtil.cpp
    FontInstaller.cpp
    GolombBuffer.cpp
    H264Nalu.cpp
    HdmvClipInfo.cpp
    MediaTypeEx.cpp
    MediaTypes.cpp
    NullRenderers.cpp
    stdafx.cpp
    text.cpp
    vd.cpp
    vd_asm.cpp
)

# mmintrin64.c is only included for x64 builds
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_sources(dsutil PRIVATE mmintrin64.c)
endif()

target_include_directories(dsutil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(dsutil PRIVATE
    ${CMAKE_SOURCE_DIR}/include/dx
)

target_link_libraries(dsutil PUBLIC BaseClasses)

target_compile_definitions(dsutil PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
)

# Assembly files - Windows Win32 only (all 3 asm files use 32-bit MASM syntax
# and are ExcludedFromBuild on x64 in the original vcxproj)
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    enable_language(ASM_MASM)
    set_source_files_properties(a_yuv2rgb.asm PROPERTIES
        LANGUAGE ASM_MASM
        COMPILE_FLAGS "/safeseh /c /coff /Cx /nologo"
    )
    set_source_files_properties(a_yuvtable.asm PROPERTIES
        LANGUAGE ASM_MASM
        COMPILE_FLAGS "/c /coff /Cx /nologo"
    )
    set_source_files_properties(convert_a.asm PROPERTIES
        LANGUAGE ASM_MASM
        COMPILE_FLAGS "/safeseh /c /coff /Cx /nologo"
    )
    target_sources(dsutil PRIVATE a_yuv2rgb.asm a_yuvtable.asm convert_a.asm)
endif()

# MFC support
if(MSVC)
    target_compile_definitions(dsutil PRIVATE
        _AFXDLL
    )
endif()
