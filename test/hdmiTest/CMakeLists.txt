pico_sdk_init()
add_executable(hdmiTest
    hdmiTest.c
    )

    target_compile_options(hdmiTest PRIVATE -Wall)

target_include_directories(libdvi INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(hdmiTest PRIVATE
    libdvi
    pico_multicore
    pico_stdlib)

pico_enable_stdio_uart(dacTest 1)