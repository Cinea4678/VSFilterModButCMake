add_library(subpic STATIC
    CoordGeom.cpp
    ISubPic.cpp
    MemSubPic.cpp
)

# DirectX SubPic backends - Windows only
if(WIN32)
    target_sources(subpic PRIVATE
        DX7SubPic.cpp
        DX9SubPic.cpp
    )
endif()

target_include_directories(subpic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(VSFILTERMOD_BUILD_DIRECTSHOW)
    target_link_libraries(subpic PUBLIC BaseClasses)
endif()

target_compile_definitions(subpic PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
)

if(MSVC)
    target_compile_definitions(subpic PRIVATE
        $<$<CONFIG:Debug>:_AFXDLL>
    )
endif()
