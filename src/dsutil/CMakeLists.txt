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

# System libraries needed by dsutil (DirectShow GUIDs, AVI, version info).
# PUBLIC so consumers (vsfiltermod_plugin, VSFilterMod) get them automatically.
if(MSVC)
    target_link_libraries(dsutil PUBLIC
        strmiids.lib
        vfw32.lib
        Version.lib
    )
endif()

target_compile_definitions(dsutil PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
)

# Assembly files - Windows Win32 only (all 3 asm files use 32-bit MASM syntax
# and are ExcludedFromBuild on x64 in the original vcxproj).
# Use add_custom_command to invoke ml.exe directly, matching the original
# vcxproj CustomBuild approach (CMake's ASM_MASM language integration is
# unreliable with the Visual Studio generator).
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(_dsutil_asm_objs "")
    foreach(_asm_src a_yuv2rgb.asm a_yuvtable.asm convert_a.asm)
        get_filename_component(_asm_name "${_asm_src}" NAME_WE)
        set(_asm_obj "${CMAKE_CURRENT_BINARY_DIR}/${_asm_name}.obj")

        # a_yuvtable.asm does not use /safeseh
        if(_asm_src STREQUAL "a_yuvtable.asm")
            set(_safeseh_flag "")
        else()
            set(_safeseh_flag /safeseh)
        endif()

        add_custom_command(
            OUTPUT "${_asm_obj}"
            COMMAND ml ${_safeseh_flag} /c /coff /Cx /nologo
                    /Fo "${_asm_obj}" "${CMAKE_CURRENT_SOURCE_DIR}/${_asm_src}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_asm_src}"
            COMMENT "Assembling ${_asm_src}"
            VERBATIM
        )
        list(APPEND _dsutil_asm_objs "${_asm_obj}")
    endforeach()

    set_source_files_properties(${_dsutil_asm_objs} PROPERTIES
        EXTERNAL_OBJECT TRUE
        GENERATED TRUE
    )
    target_sources(dsutil PRIVATE ${_dsutil_asm_objs})
endif()

# MFC support
if(MSVC)
    target_compile_definitions(dsutil PRIVATE
        _AFXDLL
    )
endif()
