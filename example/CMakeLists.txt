cmake_minimum_required(VERSION 3.14)

project(mintex LANGUAGES C)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES barebones.c simple.c advanced.c)

# add_executable(mintex_barebones barebones.c)
# target_link_libraries(mintex_barebones mint)

# add_executable(mintex_simple simple.c)
# target_link_libraries(mintex_simple mint)

add_executable(mintex_advanced advanced.c)
target_link_libraries(mintex_advanced mint)
