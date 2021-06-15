cmake_minimum_required(VERSION 3.13)
project(conway_life)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw)

include_directories(lib)

#add_executable(conway_life main.cpp lib/glad/glad.h lib/glad/glad.c)
add_executable(conway_life mobius.cpp lib/glad/glad.h lib/glad/glad.c)
target_link_libraries(conway_life glfw OpenGL)
