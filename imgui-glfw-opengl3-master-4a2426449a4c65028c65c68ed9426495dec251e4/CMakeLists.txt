cmake_minimum_required(VERSION 3.16)
project(imgui_glfw_opengl3_cmake_project)

file(GLOB IMGUI_GLFW_OPENGL3_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backends/*.cpp
)

add_library(imgui_glfw_opengl3 ${IMGUI_GLFW_OPENGL3_SOURCE_FILES})
target_include_directories(imgui_glfw_opengl3 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/backends
)
target_link_libraries(imgui_glfw_opengl3 PUBLIC glfw)
