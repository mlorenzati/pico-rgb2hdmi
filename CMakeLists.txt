cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_rgb2hdmi VERSION "0.2.0" LANGUAGES C CXX)
configure_file(config/version.h.in  src/version.h)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

#Include directory with setup config
include_directories(config)

#include directory with generated configs
include_directories(${PROJECT_BINARY_DIR}/src)

#sources wiht libs and apps, tests separated
add_subdirectory(src)
add_subdirectory(test)