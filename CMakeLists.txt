cmake_minimum_required(VERSION 3.2)

PROJECT(test_rpc)

set(LIBRDKAFKA_HOME "/Users/shenjiafeng/workspace/cpp/librdkafka-1.6.1")

include_directories("${LIBRDKAFKA_HOME}/src" "${LIBRDKAFKA_HOME}/src-cpp")

link_directories("${LIBRDKAFKA_HOME}/src" "${LIBRDKAFKA_HOME}/src-cpp")

aux_source_directory(. DIR_SRC)

link_libraries(librdkafka.a librdkafka++.a)

add_compile_options(-std=c++11)

add_library(test_rpc SHARED ${DIR_SRC})
