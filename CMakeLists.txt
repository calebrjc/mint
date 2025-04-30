cmake_minimum_required(VERSION 3.16)

project(
    mint
    VERSION 0.0.1
    DESCRIPTION "MINT Is Not Testing! Logging and assertion utilities for C."
    LANGUAGES C)

option(MINT_DEV "Enable development features" OFF)
option(MINT_BUILD_EXAMPLES "Build examples" OFF)

if (MINT_DEV)
    set(MINT_BUILD_EXAMPLES ON)
endif ()

set(mint_sources src/mint.h src/mint.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${mint_sources})

add_library(mint)
target_sources(mint PRIVATE ${mint_sources})
target_include_directories(mint PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if (MINT_DEV)
    target_compile_options(mint PRIVATE -g -Wall -Wextra --pedantic-errors)
endif ()

if (MINT_BUILD_EXAMPLES)
    add_subdirectory(example)
endif ()
