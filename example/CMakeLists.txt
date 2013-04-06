project(example)

add_executable(${PROJECT_NAME} points.cpp)
target_link_libraries(${PROJECT_NAME} ${GMP_LIBRARIES})