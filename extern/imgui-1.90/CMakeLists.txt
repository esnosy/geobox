cmake_minimum_required(VERSION 3.16)
project(imgui_cmake_project)

file(GLOB IMGUI_SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_library(imgui STATIC
    ${IMGUI_SRC_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/backends
)
