cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog glm::glm
                                     stb_image)

target_sources(geobox PRIVATE main.cpp geobox_app.cpp geobox_app.hpp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/bin)
install(TARGETS geobox DESTINATION .)
install(DIRECTORY resources DESTINATION .)
include(CPack)
