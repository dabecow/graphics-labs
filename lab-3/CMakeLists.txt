cmake_minimum_required(VERSION 3.19)
project(opengl_2)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

add_executable(opengl_2 main.cpp)

target_link_libraries(opengl_2 ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})