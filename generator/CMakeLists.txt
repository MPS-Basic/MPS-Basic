add_executable(generate_dambreak
    generate_dambreak.cpp
)
target_include_directories(generate_dambreak PRIVATE ../eigen)
target_link_libraries(generate_dambreak PRIVATE particles)

add_executable(generate_hydrostatic
    generate_hydrostatic.cpp
)
target_include_directories(generate_hydrostatic PRIVATE ../eigen)
target_link_libraries(generate_hydrostatic PRIVATE particles)
