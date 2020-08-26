cmake_minimum_required(VERSION 3.18)
project(directvolumerendering)
add_executable(dvr main.cpp linalg.cpp util.cpp unit_cube.cpp fullscreen_quad.cpp cube_shader.cpp volume_shader.cpp)

find_package(yavin REQUIRED)
target_link_libraries(dvr yavin)
target_compile_options(dvr PRIVATE -Wall -Wextra -pedantic)

add_custom_target(run ./dvr DEPENDS dvr)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cube.frag
               ${CMAKE_CURRENT_BINARY_DIR}/cube.frag)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cube.vert
               ${CMAKE_CURRENT_BINARY_DIR}/cube.vert)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/volume.frag
               ${CMAKE_CURRENT_BINARY_DIR}/volume.frag)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/volume.vert
               ${CMAKE_CURRENT_BINARY_DIR}/volume.vert)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(dvr PRIVATE -fcolor-diagnostics)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(dvr PRIVATE -fdiagnostics-color=always)
endif()
