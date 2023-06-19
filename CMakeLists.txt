cmake_minimum_required(VERSION 3.1.4)
project(mps VERSION 0.1.0 LANGUAGES C CXX)
add_executable(${PROJECT_NAME} mps.cpp)

# eigen 
include(FetchContent)
FetchContent_Declare(
   eigen
   GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
   GIT_TAG 3.4.0
)
FetchContent_MakeAvailable(eigen)