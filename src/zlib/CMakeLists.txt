add_library(zlib STATIC
    Adler32.c
    Compress.c
    Crc32.c
    Deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    Inffast.c
    Inflate.c
    Inftrees.c
    Trees.c
    Uncompr.c
    Zutil.c
)

target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
