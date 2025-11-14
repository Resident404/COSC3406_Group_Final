# Require a modern CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(COSC3406_Group_Final LANGUAGES CXX)

# Specify project files: header files and source files
set(HDRS
    asteroid.h camera.h game.h resource.h resource_manager.h scene_graph.h scene_node.h
)

set(SRCS
    asteroid.cpp camera.cpp game.cpp main.cpp resource.cpp resource_manager.cpp scene_graph.cpp scene_node.cpp
    material_vp.glsl material_fp.glsl shiny_blue_vp.glsl shiny_blue_fp.glsl
)

# Add path name to configuration file
configure_file(path_config.h.in path_config.h)

# Add executable based on the source files
add_executable(COSC3406_Group_Final ${HDRS} ${SRCS})

# Require OpenGL library
find_package(OpenGL REQUIRED)
target_link_libraries(COSC3406_Group_Final PRIVATE OpenGL::GL)

# Other libraries needed
set(LIBRARY_PATH "" CACHE PATH "Folder with GLEW, GLFW, GLM, and SOIL libraries")

target_include_directories(COSC3406_Group_Final PRIVATE
    ${LIBRARY_PATH}/include
)

if(NOT WIN32)
    find_library(GLEW_LIBRARY GLEW)
    find_library(GLFW_LIBRARY glfw)
    find_library(SOIL_LIBRARY SOIL)
else()
    find_library(GLEW_LIBRARY glew32s HINTS ${LIBRARY_PATH}/lib)
    find_library(GLFW_LIBRARY glfw3   HINTS ${LIBRARY_PATH}/lib)
    find_library(SOIL_LIBRARY SOIL    HINTS ${LIBRARY_PATH}/lib)
endif()

target_link_libraries(COSC3406_Group_Final PRIVATE
    ${GLEW_LIBRARY}
    ${GLFW_LIBRARY}
    ${SOIL_LIBRARY}
)

# Windows-specific settings
if(WIN32)
    # Avoid ZERO_CHECK target in Visual Studio
    set(CMAKE_SUPPRESS_REGENERATION TRUE)

    # Add debug postfix for Visual Studio builds
    set_target_properties(COSC3406_Group_Final PROPERTIES DEBUG_POSTFIX _d)
endif()
