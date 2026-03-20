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
if(WIN32)
    target_include_directories(libssf PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
endif()

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

# subtitles library - core files compiled on all platforms
add_library(subtitles STATIC
    GFN.cpp
    Rasterizer.cpp
    RealTextParser.cpp
    RTS.cpp
    SSF.cpp
    STS.cpp
    TextFile.cpp
    USFSubtitles.cpp
    VobSubFile.cpp
    VobSubImage.cpp
)

# Windows-only files (DirectShow, HDMV/DVB demuxers, VOB ripper)
if(WIN32)
    target_sources(subtitles PRIVATE
        BaseSub.cpp
        CCDecoder.cpp
        CompositionObject.cpp
        DVBSub.cpp
        HdmvSub.cpp
        RenderedHdmvSubtitle.cpp
        SubtitleInputPin.cpp
        VobSubFileRipper.cpp
    )
endif()

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

# On Clang/GCC, CStringW objects have operator const wchar_t*() and can safely
# pass through variadic functions. Suppress the non-pod-varargs error.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(subtitles PRIVATE -Wno-non-pod-varargs -Wno-class-varargs)
    target_compile_options(libssf PRIVATE -Wno-non-pod-varargs -Wno-class-varargs)
endif()
