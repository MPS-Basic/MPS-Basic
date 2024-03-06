# This CMakeLists.txt is only supposed to be called from CMakeLists.txt
# in the parent directory. This does not work properly only by itself.
# The CMakelists.txt in the parent directory will build generators as well
# in addition to the main soure code, so use it insted of this CMakeLists.txt
# to build generators.

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
