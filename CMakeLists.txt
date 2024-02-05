cmake_minimum_required(VERSION 3.9)
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
target_include_directories(${PROJECT_NAME} PUBLIC eigen)

# google test 
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   # Specify the commit you depend on and update it regularly.
#   URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)
# 
# enable_testing()
# add_executable(
#     mps_test 
#     test/loader_test.cpp
# )
# target_include_directories(
#     mps_test
#     PRIVATE
#     src/
# )
# target_link_libraries(
#     mps_test
#     GTest::gtest_main
# )
# 
# include(GoogleTest)
# gtest_discover_tests(mps_test)
# # coverage
# if (CMAKE_COMPILER_IS_GNUCXX)
#     set(CMAKE_C_FLAGS_PROFILE --coverage)
#     set(CMAKE_CXX_FLAGS_PROFILE --coverage)
# endif()

# yaml-cpp
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp)