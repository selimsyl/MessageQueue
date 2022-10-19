cmake_minimum_required(VERSION 3.0.0)

project(queueTest VERSION 0.1.0)

add_executable(queueTest main.cpp)

target_link_libraries(queueTest  pthread atomic)

target_compile_features(queueTest PRIVATE cxx_std_20)