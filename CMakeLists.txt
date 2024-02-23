cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-rgb2hdmi VERSION "0.6.4" LANGUAGES C CXX)
configure_file(config/version.h.in  src/version.h)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(RBG2HDMI_HASH_SALT tY6!U3@z)
set(TEST_MODE false)
set(USE_LICENSE false)
#set(INITIAL_LICENSE "9a9c3e65cd0065a475a14a44d7253c2e24d2bb23")

add_compile_definitions(FONT_C64_8X8)

if (USE_LICENSE)
    add_compile_definitions(USE_LICENSE)
endif()

if (TEST_MODE AND USE_LICENSE)
    add_compile_definitions(TEST_MODE)
endif()

if (INITIAL_LICENSE AND USE_LICENSE)
    add_compile_definitions(INITIAL_LICENSE="${INITIAL_LICENSE}")
endif()

# Initialize the Pico SDK
pico_sdk_init()

# Comment this if you don't care about binary security
find_program(SHA1SUM NAMES sha1sum)
if (NOT SHA1SUM)
    message(STATUS "sha1sum was not found")
endif()

find_program(STR2HEX NAMES xxd)
if (NOT STR2HEX)
    message(STATUS "xxd was not found")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DRGB2HDMI_DEBUG")

#Include directory with setup config
include_directories(config)

#include directory with generated configs
include_directories(${PROJECT_BINARY_DIR}/src)

#sources wiht libs and apps, tests separated
add_subdirectory(src)
add_subdirectory(test)