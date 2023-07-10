cmake_minimum_required(VERSION 3.1.4)
project(mps VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_SOURCE_DIR src/)
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/mps.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17) # c++17 for simulation code

# eigen 
include_directories(eigen)