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
  src/particles_loader/prof.cpp
  src/particles_loader/csv.cpp
  src/refvalues.cpp
  src/saver.cpp
  src/simulation.cpp
  src/weight.cpp
  src/pressure_calculator/implicit.cpp
  src/pressure_calculator/explicit.cpp
  src/pressure_calculator/pressure_poisson_equation.cpp
  src/pressure_calculator/dirichlet_boundary_condition.cpp
  src/pressure_calculator/dirichlet_boundary_condition_generator/free_surface.cpp
  src/surface_detector/number_density.cpp
  src/surface_detector/distribution.cpp
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
  Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)
if (NOT eigen_POPULATED)
  message(STATUS "Fetching Eigen...")
  FetchContent_Populate(Eigen)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${eigen_SOURCE_DIR})

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
    src/particle.cpp
    src/particles.cpp
    src/particles_exporter.cpp
    test/particles_exporter_test.cpp
)
target_link_libraries( # required for FRIEND_TEST macro to test private members
    ${PROJECT_NAME}
    PRIVATE
    GTest::gtest_main
)
target_include_directories(
    mps_test
    PRIVATE
    src/
)
target_include_directories(
  mps_test 
  PRIVATE
  ${eigen_SOURCE_DIR}
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

# -------------------------------
# ----- fast-cpp-csv-parser -----
# -------------------------------
FetchContent_Declare(
  fast-cpp-csv-parser
  GIT_REPOSITORY https://github.com/ben-strasser/fast-cpp-csv-parser.git
  GIT_TAG a71a87e700b0fb92645c6b124742cbf326e0f7b1
)

if(NOT fast-cpp-csv-parser_POPULATED)
  message(STATUS "Fetching fast-cpp-csv-parser...")
  FetchContent_Populate(fast-cpp-csv-parser)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${fast-cpp-csv-parser_SOURCE_DIR})

# ------------------------------
# ----- particle generator -----
# ------------------------------
add_library(particles STATIC
  src/particle.cpp
  src/particles.cpp
  src/particles_exporter.cpp
)
target_include_directories(particles PRIVATE ${eigen_SOURCE_DIR})
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
