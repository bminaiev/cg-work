project(practice-1)

add_executable(find-points find-points.cpp)
target_link_libraries(find-points ${GMP_LIBRARIES})