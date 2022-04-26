if (TARGET tinyusb_device)
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

    target_link_libraries(hdmiTest PRIVATE
        libdvi
        pico_multicore
        pico_stdlib)

    pico_enable_stdio_usb(hdmiTest 1)
    pico_enable_stdio_uart(hdmiTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building hdmiTest because TinyUSB submodule is not initialized in the SDK")
endif()
