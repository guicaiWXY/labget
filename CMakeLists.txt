cmake_minimum_required(VERSION 3.6)
project(labget)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Lpcre -lpcrecpp -lpcreposix")

set(SOURCE_FILES main.cpp labget.h dns.cpp dns.h io.cpp io.h uri.h uri.cpp http.cpp http.h)

#add_compile_options(-l/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lpcre)

add_executable(labget ${SOURCE_FILES})
#add_library(pcre3 STATIC IMPORTED)
#set_target_properties(pcre3 PROPERTIES
#        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/libbaz.a
#        IMPORTED_LOCATION_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/libbazd.a)