set(CMAKE_CXX_STANDARD 17)
add_executable(generate_dambreak generate_dambreak.cpp)
target_include_directories(generate_dambreak PRIVATE ../eigen)