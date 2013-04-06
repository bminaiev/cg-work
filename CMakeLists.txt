cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(cg-work)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cg/CMake/Modules)

add_subdirectory(cg)

find_package(GMP)

include_directories("cg/include" ${GMP_INCLUDE_DIR})

add_subdirectory(practice-1)
add_subdirectory(example)