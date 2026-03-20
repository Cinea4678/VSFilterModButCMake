# libssf (SSF subtitle format library)
add_library(libssf STATIC
    libssf/Arabic.cpp
    libssf/Array.cpp
    libssf/Exception.cpp
    libssf/File.cpp
    libssf/FontWrapper.cpp
    libssf/Glyph.cpp
    libssf/GlyphPath.cpp
    libssf/Node.cpp
    libssf/NodeFactory.cpp
    libssf/Rasterizer.cpp
    libssf/Renderer.cpp
    libssf/Split.cpp
    libssf/Stream.cpp
    libssf/StringMap.cpp
    libssf/Subtitle.cpp
    libssf/SubtitleFile.cpp
)

target_include_directories(libssf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libssf
)
target_include_directories(libssf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

if(VSFILTERMOD_BUILD_DIRECTSHOW)
    target_link_libraries(libssf PUBLIC BaseClasses)
endif()

target_compile_definitions(libssf PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
)

if(MSVC)
    target_compile_definitions(libssf PRIVATE
        $<$<CONFIG:Debug>:_AFXDLL>
    )
endif()

# subtitles library
add_library(subtitles STATIC
    BaseSub.cpp
    CCDecoder.cpp
    CompositionObject.cpp
    DVBSub.cpp
    GFN.cpp
    HdmvSub.cpp
    Rasterizer.cpp
    RealTextParser.cpp
    RenderedHdmvSubtitle.cpp
    RTS.cpp
    SSF.cpp
    STS.cpp
    SubtitleInputPin.cpp
    TextFile.cpp
    USFSubtitles.cpp
    VobSubFile.cpp
    VobSubFileRipper.cpp
    VobSubImage.cpp
)

target_include_directories(subtitles PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(subtitles PUBLIC
    libssf
    libpng
    zlib
)

if(VSFILTERMOD_BUILD_DIRECTSHOW)
    target_link_libraries(subtitles PUBLIC BaseClasses)
endif()

target_compile_definitions(subtitles PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
    _VSMOD
)

if(MSVC)
    target_compile_definitions(subtitles PRIVATE
        $<$<CONFIG:Debug>:_AFXDLL>
    )
endif()
