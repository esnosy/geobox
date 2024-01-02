cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog glm::glm
                                     stb_image)

target_sources(geobox PRIVATE main.cpp geobox_app.cpp geobox_app.hpp)

# TODO: maybe use a more efficient method, one that doesn't delete and recopy
# files over and over?
add_custom_target(
  copy_resources ALL
  COMMAND ${CMAKE_COMMAND} -E remove_directory
          ${CMAKE_CURRENT_BINARY_DIR}/resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources
          ${CMAKE_CURRENT_BINARY_DIR}/resources
  COMMENT "Copying resources")
add_dependencies(geobox copy_resources)
