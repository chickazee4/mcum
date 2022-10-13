cmake_minimum_required(VERSION 3.1)
project(mcum)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)
set(CFGFILE "${CMAKE_INSTALL_PREFIX}/etc/mcum/config.json" CACHE STRING "Location of the bot configuration file")

find_package(CURL REQUIRED)
find_package(JSON-C REQUIRED)
find_library(
    CONCORD
    NAMES libdiscord.a libdiscord.so
    HINTS /lib64 /usr/lib64 /lib /usr/lib /usr/local/lib /usr/local/lib64
)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_executable(mcum main.c imgflip.c password.c arrays.c)
target_link_libraries(mcum ${CONCORD} CURL::libcurl ${JSON-C_LIBRARY} pthread)
target_include_directories(mcum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
