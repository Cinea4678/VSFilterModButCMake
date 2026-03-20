# Cross-platform VapourSynth + AviSynth plugin
# Builds a shared library (.dll on Windows, .dylib on macOS, .so on Linux)

add_library(vsfiltermod_plugin SHARED
    plugins_xplat.cpp
    vfr_xplat.cpp
)

target_include_directories(vsfiltermod_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vapoursynth/sdk/include/vapoursynth
)

target_link_libraries(vsfiltermod_plugin PRIVATE
    subtitles
    subpic
    platform
    libpng
    zlib
)

target_compile_definitions(vsfiltermod_plugin PRIVATE
    _VSMOD
)

# On non-Windows, link pthread and dl
if(NOT WIN32)
    target_link_libraries(vsfiltermod_plugin PRIVATE
        pthread
        dl
    )
endif()

# On macOS, set dylib install name
if(APPLE)
    set_target_properties(vsfiltermod_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Output name without "lib" prefix for plugin loading
set_target_properties(vsfiltermod_plugin PROPERTIES
    OUTPUT_NAME "vsfiltermod"
    PREFIX ""
)
