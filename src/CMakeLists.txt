cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog glm::glm stb_image)

target_sources(geobox PRIVATE main.cpp geobox_app.cpp geobox_app.hpp)

set(RESOURCES_TARGET_DIR $<TARGET_FILE_DIR:geobox>/resources)
# TODO: more efficient syncing of resources directory
add_custom_target(
        copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RESOURCES_TARGET_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${RESOURCES_TARGET_DIR}
        COMMENT "Copying resources into binary directory"
)

install(TARGETS geobox DESTINATION .)
install(DIRECTORY resources DESTINATION .)
include(CPack)
