pico_sdk_init()
add_executable(afeTest
    afeTest.c
)

target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(afeTest PRIVATE
    rgbScan
    pico_multicore
    pico_stdlib)

pico_enable_stdio_uart(afeTest 1)