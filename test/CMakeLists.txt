cmake_minimum_required(VERSION 3.2)

PROJECT(service_demo)

set(MASHED_TARO_HOME "/Users/shenjiafeng/workspace/cpp/mashed_taro")

include_directories("${MASHED_TARO_HOME}")

link_directories("${MASHED_TARO_HOME}/build")

link_libraries(test_rpc)

add_compile_options(-std=c++11 -g)

add_executable(service_demo service_demo.cpp)
add_executable(client_demo client_demo.cpp)
