cmake_minimum_required(VERSION 2.8)
project(mm)

set(CMAKE_INSTALL_PREFIX ../bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. SRCS)
aux_source_directory(src SRCS)
aux_source_directory(src/Automaton SRCS)
aux_source_directory(src/Network SRCS)
aux_source_directory(src/Protocol SRCS)
aux_source_directory(src/SearchEngine SRCS)
aux_source_directory(src/ThreadPool SRCS)

add_executable(mm ${SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O3 -g -minline-all-stringops -pipe -Wall -Werror -Wno-deprecated -Wno-unused-variable -Werror=strict-aliasing -lpthread -levent")