cmake_minimum_required(VERSION 3.12)
project(mps VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# main executable
add_executable(${PROJECT_NAME}
  src/bucket.cpp
  src/loader.cpp
  src/main.cpp
  src/mps.cpp
  src/particle.cpp
  src/particles.cpp
  src/particles_exporter.cpp
  src/refvalues.cpp
  src/saver.cpp
  src/simulation.cpp
  src/weight.cpp
  src/pressure_calculator/implicit.cpp
  src/pressure_calculator/explicit.cpp
  src/pressure_calculator/pressure_poisson_equation.cpp
  src/pressure_calculator/target_id_map.cpp
  src/neighbor_searcher.cpp
)

# ------------------
# ----- OpenMP -----
# ------------------
find_package(OpenMP 5.0)
if(OpenMP_CXX_FOUND)
  target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
  message(WARNING "OpenMP not found. It runs on single thread.")
endif()

# -----------------
# ----- Eigen -----
# -----------------
include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)
FetchContent_MakeAvailable(Eigen)
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

# -----------------------
# ----- Google Test -----
# -----------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(
    mps_test
    src/refvalues.cpp
    test/refvalues_test.cpp
    src/weight.cpp
    test/weight_test.cpp
)
target_include_directories(
    mps_test
    PRIVATE
    src/
)
target_link_libraries(
    mps_test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mps_test)
# # coverage
# if (CMAKE_COMPILER_IS_GNUCXX)
#     set(CMAKE_C_FLAGS_PROFILE --coverage)
#     set(CMAKE_CXX_FLAGS_PROFILE --coverage)
# endif()

# --------------------
# ----- yaml-cpp -----
# --------------------
include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp)

# ------------------------------
# ----- particle generator -----
# ------------------------------
add_library(particles STATIC
  src/particle.cpp
  src/particles.cpp
  src/particles_exporter.cpp
)
target_link_libraries(particles PUBLIC Eigen3::Eigen)
add_subdirectory(generator)

# --------------------
# ----- argparse -----
# --------------------
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v3.0
)
FetchContent_MakeAvailable(argparse)
target_link_libraries(${PROJECT_NAME} PUBLIC argparse)
