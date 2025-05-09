cmake_minimum_required(VERSION 3.14)

project(mintex LANGUAGES C)

set(mintex_sources main.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${mintex_sources})

add_executable(mintex ${mintex_sources})
target_link_libraries(mintex mint)
