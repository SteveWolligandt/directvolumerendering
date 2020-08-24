cmake_minimum_required(VERSION 3.18)
project(directvolumerendering)
add_executable(dvr main.cpp)

find_package(yavin REQUIRED)
target_link_libraries(dvr yavin)
