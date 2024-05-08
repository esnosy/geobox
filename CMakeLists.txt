cmake_minimum_required(VERSION 3.16)
project(geobox_cmake_project)

option(ENABLE_CCACHE "" YES)

if(ENABLE_CCACHE)
    message(STATUS "Enabling ccache")

    # https://cliutils.gitlab.io/modern-cmake/chapters/features/utilities.html#ccache
    find_program(CCACHE_PROGRAM ccache REQUIRED)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

option(ENABLE_ASAN "" YES)

if (ENABLE_ASAN)
    add_compile_definitions(GEOPROC_ASAN)
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/INCREMENTAL:NO) # Disable incremental linking because MSVC ASAN is not compatible with it.
    else ()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif ()
endif ()

add_subdirectory(extern)
add_subdirectory(src)
