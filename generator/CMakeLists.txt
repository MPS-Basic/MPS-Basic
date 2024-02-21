add_executable(generate_dambreak
    generate_dambreak.cpp
)
target_include_directories(generate_dambreak PRIVATE ../eigen)#/home/queuesys/Desktop/MPS-Basic/src/bucket.hpp
target_link_libraries(generate_dambreak PRIVATE particles)