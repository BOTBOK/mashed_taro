cmake_minimum_required(VERSION 3.2)

PROJECT(test_rpc)

set(LIBRDKAFKA_HOME "/home/oldman/workspace/librdkafka-1.6.1")

include_directories("${LIBRDKAFKA_HOME}/src" "${LIBRDKAFKA_HOME}/src-cpp")

link_directories("${LIBRDKAFKA_HOME}/src" "${LIBRDKAFKA_HOME}/src-cpp")

link_libraries(librdkafka++.a librdkafka.a)
link_libraries(libdl.so libpthread.so)
aux_source_directory(. DIR_SRC)

add_compile_options(-std=c++11 -g)

add_library(test_rpc SHARED ${DIR_SRC})