add_executable(generate_dambreak
    generate_dambreak.cpp
)
target_link_libraries(generate_dambreak PUBLIC Eigen3::Eigen)
target_link_libraries(generate_dambreak PRIVATE particles)

add_executable(generate_hydrostatic
    generate_hydrostatic.cpp
)
target_link_libraries(generate_hydrostatic PUBLIC Eigen3::Eigen)
target_link_libraries(generate_hydrostatic PRIVATE particles)
