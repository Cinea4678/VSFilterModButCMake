add_library(platform STATIC
    font_engine.h
)

# Empty SOURCES trick - header-only on Windows for now,
# FreeType implementation on non-Windows
if(WIN32)
    target_sources(platform PRIVATE font_engine_gdi.cpp)
else()
    target_sources(platform PRIVATE font_engine_freetype.cpp)
    target_link_libraries(platform PUBLIC
        Freetype::Freetype
        PkgConfig::HARFBUZZ
        PkgConfig::FONTCONFIG
    )
endif()

target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
