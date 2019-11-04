cmake_minimum_required(VERSION 2.8)
project(mm)

set(CMAKE_INSTALL_PREFIX ../bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(src SRCS)
file(GLOB dirs src/*)
FOREACH(dir ${dirs})
    IF (IS_DIRECTORY ${dir})
        aux_source_directory(${dir} SRCS)
    ENDIF()
ENDFOREACH()

add_executable(mm ${SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -g -minline-all-stringops -pipe -Wall -Werror -Wno-deprecated -Wno-unused-variable -Werror=strict-aliasing -lpthread -ldl -levent")