cmake_minimum_required(VERSION 3.16)
project(geobox_src_cmake_project)

add_executable(geobox)
target_compile_features(geobox PRIVATE cxx_std_20)
set_target_properties(geobox PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(geobox PRIVATE
        /we4716 # Equivalent to Clang/GCC -Werror=return-type # TODO: confirm
        /we4834 # Equivalent to Clang/GCC -Werror=unused-result # TODO: confirm
    )
else()
    target_compile_options(geobox PRIVATE -Werror=return-type -Werror=unused-result)
endif()

option(ENABLE_SUPERLUMINAL_PERF_API "" NO)
set(SUPERLUMINAL_PERF_API_DIR "C:\\Program Files\\Superluminal\\Performance\\API" CACHE STRING "")

if(ENABLE_SUPERLUMINAL_PERF_API)
    set(CMAKE_MODULE_PATH ${SUPERLUMINAL_PERF_API_DIR} ${CMAKE_MODULE_PATH})
    find_package(SuperluminalAPI REQUIRED)
    target_link_libraries(geobox PRIVATE SuperluminalAPI)
    # We intent here to define ENABLE_SUPERLUMINAL_PERF_API and not its value (ON or OFF),
    # thankfully this is how CMake behaves as of CMake 3.28.3 with 3.16 specified as the minimum version required.
    target_compile_definitions(geobox PRIVATE ENABLE_SUPERLUMINAL_PERF_API)
endif()

target_link_libraries(geobox PRIVATE glad glfw imgui ImGuiFileDialog glm::glm stb_image)
target_sources(geobox PRIVATE
    main.cpp
    geobox_app.cpp
    geobox_app.hpp
    bvh.cpp
    bvh.hpp
    orbit_camera.cpp
    orbit_camera.hpp
    common.hpp
    indexed_triangle_mesh_object.cpp
    indexed_triangle_mesh_object.hpp
    read_stl.cpp
    read_stl.hpp
    aabb.hpp
    ray.hpp
    ray_aabb_intersection.cpp
    ray_aabb_intersection.hpp
    point_cloud_object.cpp
    point_cloud_object.hpp
    shader.cpp
    shader.hpp
    intersection.cpp
    intersection.hpp
    math.cpp
    math.hpp
    primitives.cpp
    primitives.hpp
    random_generator.hpp
)

# https://web.archive.org/web/20240419204531/https://cliutils.gitlab.io/modern-cmake/chapters/features/small.html#interprocedural-optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT is_ipo_supported)
if (is_ipo_supported)
    if (MSVC AND ENABLE_ASAN)
        # Default CMake inter-procedural optimization MSVC flags are not compatible with ASAN
        # https://gitlab.kitware.com/cmake/cmake/-/issues/20484
        add_link_options(/LTCG) # Enable "Link Time Code Generation"
        add_compile_options(/GL) # LTCG does nothing without "Whole Program Optimization"
        add_link_options(/INCREMENTAL:NO) # Disable incremental linking because /LTCG is not compatible with it.
    else ()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
endif ()

set(RESOURCES_TARGET_DIR $<TARGET_FILE_DIR:geobox>/resources)

# TODO: more efficient syncing of resources directory
# TODO: avoid having to manually tun the "copy_resources" target
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
    ray_aabb_intersection.cpp
    ray_aabb_intersection.hpp
    math.cpp
    math.hpp
    common.hpp
)
target_link_libraries(test_ray_aabb_intersection PRIVATE glm::glm)
target_compile_features(test_ray_aabb_intersection PRIVATE cxx_std_20)
set_target_properties(test_ray_aabb_intersection PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(test_ray_aabb_intersection PRIVATE TEST_RAY_AABB_INTERSECTION)

add_executable(test_intersection
    intersection.cpp
    intersection.hpp
    math.cpp
    math.hpp
)
target_link_libraries(test_intersection PRIVATE glm::glm)
target_compile_features(test_intersection PRIVATE cxx_std_20)
set_target_properties(test_intersection PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(test_intersection PRIVATE GEOBOX_TEST_INTERSECTION)
