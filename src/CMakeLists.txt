cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog)

target_sources(geobox
    PRIVATE
    main.cpp
    geobox_app.cpp
    geobox_app.hpp
    vec3f.hpp
)

# TODO: maybe use a more efficient method, one that doesn't delete and recopy files over and over?
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_LIST_DIR}/shaders
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMENT "Copying shaders"
)
add_dependencies(geobox copy_shaders)

option(WITH_ASAN "Enable ASAN" YES)

if(WITH_ASAN)
    if(MSVC)
        target_compile_options(glad PRIVATE "/fsanitize=address")
        target_compile_options(glfw PRIVATE "/fsanitize=address")
        target_compile_options(imgui PRIVATE "/fsanitize=address")
        target_compile_options(ImGuiFileDialog PRIVATE "/fsanitize=address")
        target_compile_options(geobox PRIVATE "/fsanitize=address")
    else()
        target_compile_options(glad PRIVATE "-fsanitize=address")
        target_compile_options(glfw PRIVATE "-fsanitize=address")
        target_compile_options(imgui PRIVATE "-fsanitize=address")
        target_compile_options(ImGuiFileDialog PRIVATE "-fsanitize=address")
        target_compile_options(geobox PRIVATE "-fsanitize=address")

        target_link_options(glad PRIVATE "-fsanitize=address")
        target_link_options(glfw PRIVATE "-fsanitize=address")
        target_link_options(imgui PRIVATE "-fsanitize=address")
        target_link_options(ImGuiFileDialog PRIVATE "-fsanitize=address")
        target_link_options(geobox PRIVATE "-fsanitize=address")
    endif()
endif()