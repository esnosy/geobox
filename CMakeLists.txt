cmake_minimum_required(VERSION 3.16)
project(geobox_cmake_project)

option(WITH_ASAN "Enable ASAN" YES)

if(WITH_ASAN)
    message(STATUS "Enabling ASAN")

    if(MSVC)
        add_compile_options("/fsanitize=address")
    else()
        add_compile_options("-fsanitize=address")
        add_link_options("-fsanitize=address")
    endif()
endif()

add_subdirectory(extern)
add_subdirectory(src)
