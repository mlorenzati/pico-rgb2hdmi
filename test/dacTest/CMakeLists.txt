pico_sdk_init()
add_executable(dacTest
    dacTest.c
    )

target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(dacTest PRIVATE
    rgbScan
    pico_multicore
    pico_stdlib)

pico_enable_stdio_uart(dacTest 1)