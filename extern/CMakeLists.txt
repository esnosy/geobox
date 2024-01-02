cmake_minimum_required(VERSION 3.16)
project(geobox_extern_cmake_project)

add_subdirectory(glad)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw-3.3.9)

add_subdirectory(imgui-1.90)
target_link_libraries(imgui PRIVATE glfw)

add_subdirectory(ImGuiFileDialog-0.6.6.1)
target_link_libraries(ImGuiFileDialog PRIVATE imgui)

set(GLM_BUILD_TESTS OFF)
# Setting BUILD_SHARED_LIBS for subdirectory only:
# https://stackoverflow.com/a/31140797/8094047
set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(glm-0.9.9.8)
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")
