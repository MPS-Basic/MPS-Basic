cmake_minimum_required(VERSION 3.1.4)
project(mps VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# main executable
add_executable(${PROJECT_NAME} src/mps.cpp)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()
# eigen 
add_subdirectory(eigen)
target_include_directories(${PROJECT_NAME} PUBLIC eigen)

# test section
add_executable(mps_test test/loader_test.cpp)
# google test 
add_subdirectory(googletest)
option(INSTALL_GTEST "Enables installation of googletest" OFF) # Disable install
target_include_directories(mps_test PUBLIC googletest/googletest/include)
target_include_directories(mps_test PUBLIC src)
target_link_libraries(mps_test gtest_main)
# test coverage
target_compile_options(mps_test PUBLIC -fprofile-arcs -ftest-coverage)
target_link_options(mps_test PUBLIC -lgcov --coverage)
enable_testing()