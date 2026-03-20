add_library(basevideofilter STATIC
    BaseVideoFilter.cpp
)

target_include_directories(basevideofilter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(basevideofilter PUBLIC BaseClasses)

target_compile_definitions(basevideofilter PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32>
)

# MFC support
if(MSVC)
    target_compile_definitions(basevideofilter PRIVATE
        $<$<CONFIG:Debug>:_AFXDLL>
    )
endif()
