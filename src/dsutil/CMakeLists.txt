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

# Assembly files - Windows only with MASM/YASM
if(MSVC)
    enable_language(ASM_MASM)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # Win32: use MASM (ml)
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
    else()
        # x64: a_yuvtable and convert_a use ml64, a_yuv2rgb uses yasm
        set_source_files_properties(a_yuvtable.asm PROPERTIES
            LANGUAGE ASM_MASM
            COMPILE_FLAGS "/c /coff /Cx /nologo"
        )
        set_source_files_properties(convert_a.asm PROPERTIES
            LANGUAGE ASM_MASM
            COMPILE_FLAGS "/c /coff /Cx /nologo"
        )
        target_sources(dsutil PRIVATE a_yuvtable.asm convert_a.asm)

        # a_yuv2rgb.asm on x64 uses yasm - add as custom command
        find_program(YASM_EXECUTABLE yasm)
        if(YASM_EXECUTABLE)
            set(YUV2RGB_OBJ "${CMAKE_CURRENT_BINARY_DIR}/a_yuv2rgb.obj")
            add_custom_command(
                OUTPUT ${YUV2RGB_OBJ}
                COMMAND ${YASM_EXECUTABLE} -X vc -g cv8 -f win64
                    -o ${YUV2RGB_OBJ}
                    ${CMAKE_CURRENT_SOURCE_DIR}/a_yuv2rgb.asm
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/a_yuv2rgb.asm
                COMMENT "Assembling a_yuv2rgb.asm with YASM"
            )
            target_sources(dsutil PRIVATE ${YUV2RGB_OBJ})
            set_source_files_properties(${YUV2RGB_OBJ} PROPERTIES
                EXTERNAL_OBJECT TRUE
                GENERATED TRUE
            )
        else()
            message(WARNING "YASM not found - a_yuv2rgb.asm will not be assembled for x64")
        endif()
    endif()
endif()

# MFC support
if(MSVC)
    target_compile_definitions(dsutil PRIVATE
        $<$<CONFIG:Debug>:_AFXDLL>
    )
endif()
