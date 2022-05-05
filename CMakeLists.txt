cmake_minimum_required(VERSION 3.10)

# Set Project Name
project(AphroditesRenderingEngine VERSION 1.0)

# configure_file()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# add the AphroditesRenderingEngine library
include_directories( ${OPENGL_INCLUDE_DIRS} )
set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
add_subdirectory(glfw)

add_subdirectory(glm)

add_subdirectory(AphroditesRenderingEngine)
add_subdirectory(Demo)