add_library(zlib STATIC
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)

target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# On non-Windows, zlib needs POSIX APIs (lseek, write, close)
if(NOT WIN32)
    target_compile_definitions(zlib PRIVATE _LARGEFILE64_SOURCE=1)
    # Suppress implicit function declaration errors for POSIX functions (lseek, write, close)
    target_compile_options(zlib PRIVATE -Wno-error=implicit-function-declaration -Wno-implicit-function-declaration)
endif()
