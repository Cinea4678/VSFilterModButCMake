add_library(VSFilterMod SHARED
    Copy.cpp
    csriapi.cpp
    DirectVobSub.cpp
    DirectVobSubFilter.cpp
    DirectVobSubPropPage.cpp
    plugins.cpp
    StdAfx.cpp
    StyleEditorDialog.cpp
    systray.cpp
    TextInputPin.cpp
    vfr.cpp
    VSFilter.cpp
)

target_include_directories(VSFilterMod PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/BaseClasses
    ${CMAKE_SOURCE_DIR}/src/libpng
    ${CMAKE_SOURCE_DIR}/src/zlib
    ${CMAKE_SOURCE_DIR}/vapoursynth/sdk/include/vapoursynth
)

target_link_libraries(VSFilterMod PRIVATE
    subtitles
    subpic
    dsutil
    basevideofilter
    BaseClasses
    libssf
    libpng
    zlib
)

target_compile_definitions(VSFilterMod PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
    ISOLATION_AWARE_ENABLED
    _WIN32_WINNT=0x0600
    _VSMOD
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_definitions(VSFilterMod PRIVATE
        _AFXDLL
    )

    # Module definition file for exports
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_sources(VSFilterMod PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/VSFilterMod.def)
    else()
        target_sources(VSFilterMod PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/VSFilterMod64.def)
    endif()

    # Resource file
    target_sources(VSFilterMod PRIVATE VSFilterMod.rc)

    # Linker flags
    target_link_options(VSFilterMod PRIVATE
        /SUBSYSTEM:WINDOWS
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        "/MERGE:_RDATA=.rdata"
        "/MERGE:_rdata=.rdata"
        "/MERGE:_TEXT64=.text"
    )

    # System libraries
    target_link_libraries(VSFilterMod PRIVATE
        Winmm.lib
        strmiids.lib
        vfw32.lib
        Version.lib
    )
endif()

# Resource compiler definitions
if(MSVC)
    set_source_files_properties(VSFilterMod.rc PROPERTIES
        COMPILE_DEFINITIONS "_AFXDLL;NDEBUG;AFX_RESOURCE_DLL;AFX_TARG_ENU"
    )
endif()

# Output name without 'lib' prefix on all platforms
set_target_properties(VSFilterMod PROPERTIES
    PREFIX ""
    OUTPUT_NAME "VSFilterMod"
)
