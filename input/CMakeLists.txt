cmake_minimum_required(VERSION 3.9)
project(make_input)
add_executable(make_input make_input.cpp)
target_include_directories(make_input PRIVATE ../eigen)