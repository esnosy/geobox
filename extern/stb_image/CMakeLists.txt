cmake_minimum_required(VERSION 3.16)
project(stb_image_cmake_project)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_LIST_DIR})
