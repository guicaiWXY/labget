cmake_minimum_required(VERSION 3.6)
project(labget)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp labget.h dns.cpp dns.h io.cpp io.h uri.h uri.cpp http.cpp http.h gzip.cpp gzip.h
        include/zlib/adler32.c
        include/zlib/compress.c
        include/zlib/crc32.c
        include/zlib/crc32.h
        include/zlib/deflate.c
        include/zlib/deflate.h
        include/zlib/gzclose.c
        include/zlib/gzguts.h
        include/zlib/gzlib.c
        include/zlib/gzread.c
        include/zlib/gzwrite.c
        include/zlib/infback.c
        include/zlib/inffast.c
        include/zlib/inffast.h
        include/zlib/inffixed.h
        include/zlib/inflate.c
        include/zlib/inflate.h
        include/zlib/inftrees.c
        include/zlib/inftrees.h
        include/zlib/trees.c
        include/zlib/trees.h
        include/zlib/uncompr.c
        include/zlib/zconf.h
        include/zlib/zlib.h
        include/zlib/zutil.c
        include/zlib/zutil.h)
#FILE(GLOB include/zlib/*)

#add_compile_options(-pthread -lpthread )

add_executable(labget ${SOURCE_FILES})
#add_library(pcre3 STATIC IMPORTED)
#set_target_properties(pcre3 PROPERTIES
#        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/libbaz.a
#        IMPORTED_LOCATION_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/libbazd.a)