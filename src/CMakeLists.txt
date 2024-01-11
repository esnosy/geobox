cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

option(ENABLE_SUPERLUMINAL_PERF_API "" NO)
if (${ENABLE_SUPERLUMINAL_PERF_API})
    set(CMAKE_MODULE_PATH "C:\\Program Files\\Superluminal\\Performance\\API" ${CMAKE_MODULE_PATH})
    find_package(SuperluminalAPI REQUIRED)
    target_link_libraries(geobox PRIVATE SuperluminalAPI)
    target_compile_definitions(geobox PRIVATE ENABLE_SUPERLUMINAL_PERF_API)
endif ()

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog glm::glm stb_image)
target_sources(geobox PRIVATE main.cpp geobox_app.cpp geobox_app.hpp bvh.cpp bvh.hpp)

# Enable Inter-procedural Optimization (IPO), also known as Link-Time optimization (LTO)
include(CheckIPOSupported)
check_ipo_supported(RESULT is_ipo_supported)
if (is_ipo_supported)
    message(STATUS "Enabling Link-Time Optimization")
    set_target_properties(geobox PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

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


add_executable(test_ray_aabb_intersection
        bvh.cpp
        bvh.hpp
)
target_link_libraries(test_ray_aabb_intersection PRIVATE glm::glm)
target_compile_features(test_ray_aabb_intersection PRIVATE cxx_std_20)
set_target_properties(test_ray_aabb_intersection PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(test_ray_aabb_intersection PRIVATE TEST_RAY_AABB_INTERSECTION)
