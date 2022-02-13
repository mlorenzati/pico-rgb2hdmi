pico_sdk_init()
add_executable(hdmiTest
    hdmiTest.c
    )

target_compile_options(hdmiTest PRIVATE -Wall)

target_compile_definitions(hdmiTest PRIVATE
	DVI_VERTICAL_REPEAT=1
	DVI_N_TMDS_BUFFERS=5
	DVI_SYMBOLS_PER_WORD=1
	)

target_include_directories(libdvi INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(hdmiTest PRIVATE
    libdvi
    pico_multicore
    pico_stdlib)

pico_enable_stdio_uart(hdmiTest 1)