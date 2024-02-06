cmake_minimum_required(VERSION 3.16)
project(geobox_extern_cmake_project)

set(GLAD_SRC_DIR glad)
add_library(glad ${GLAD_SRC_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_SRC_DIR}/include)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw-3.3.9)

set(IMGUI_SRC_DIR imgui-1.90.1-docking)
add_library(
    imgui STATIC
    ${IMGUI_SRC_DIR}/imgui.cpp
    ${IMGUI_SRC_DIR}/imgui_demo.cpp
    ${IMGUI_SRC_DIR}/imgui_draw.cpp
    ${IMGUI_SRC_DIR}/imgui_tables.cpp
    ${IMGUI_SRC_DIR}/imgui_widgets.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_SRC_DIR} ${IMGUI_SRC_DIR}/backends)
target_link_libraries(imgui PRIVATE glfw)

add_subdirectory(ImGuiFileDialog-iyad-5aca36b)
target_link_libraries(ImGuiFileDialog PRIVATE imgui)

set(GLM_BUILD_TESTS OFF)

# Setting BUILD_SHARED_LIBS for subdirectory only: https://stackoverflow.com/a/31140797/8094047
set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(glm-0.9.9.8)
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")

add_subdirectory(stb_image)
