cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_rgb2hdmi VERSION "0.3.3" LANGUAGES C CXX)
configure_file(config/version.h.in  src/version.h)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(RBG2HDMI_HASH_SALT tY6!U3@z)
set(TEST_MODE false)
set(PURCHASE_MODE false)
#set(INITIAL_LICENSE "{0xbe, 0x7c, 0x7a, 0xdf, 0x8b, 0xf0, 0xd1, 0x93, 0xa1, 0x73, 0x50, 0xa4, 0xea, 0xe3, 0xf7, 0x72, 0xa1, 0x1b, 0xe2, 0x45}")

add_compile_definitions(FONT_C64_8X8)
if (TEST_MODE)
    add_compile_definitions(TEST_MODE)
endif()

if (PURCHASE_MODE)
    add_compile_definitions(PURCHASE_MODE RGB_SCANNER_TIMING_INFO)
endif()

if (INITIAL_LICENSE)
    add_compile_definitions(INITIAL_LICENSE=${INITIAL_LICENSE})
endif()

# Initialize the Pico SDK
pico_sdk_init()

# Comment this if you don't care about binary security
find_program(SHA1SUM NAMES sha1sum)
if (NOT SHA1SUM)
    message(FATAL_ERROR "sha1sum was not found")
endif()

find_program(STR2HEX NAMES xxd)
if (NOT STR2HEX)
    message(FATAL_ERROR "xxd was not found")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DRGB2HDMI_DEBUG")

#Include directory with setup config
include_directories(config)

#include directory with generated configs
include_directories(${PROJECT_BINARY_DIR}/src)

#sources wiht libs and apps, tests separated
add_subdirectory(src)
add_subdirectory(test)