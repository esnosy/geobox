cmake_minimum_required(VERSION 3.16)
project(geobox_cmake_project)

add_executable(geobox geobox.cpp)
target_compile_features(geobox PRIVATE cxx_std_11)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw-3.3.9)

add_subdirectory(glad)
target_link_libraries(geobox PRIVATE glad)

add_subdirectory(imgui-glfw-opengl3-master-4a2426449a4c65028c65c68ed9426495dec251e4)
target_link_libraries(geobox PRIVATE imgui_glfw_opengl3)
