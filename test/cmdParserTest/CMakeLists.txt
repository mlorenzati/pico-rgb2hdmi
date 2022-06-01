if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(cmdParserTest
        cmdParserTest.c
        )

    target_link_libraries(cmdParserTest PRIVATE
        pico_multicore
        pico_stdlib
        cmdParser)
        
    pico_enable_stdio_usb(cmdParserTest 1)
    pico_enable_stdio_uart(cmdParserTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building cmdParserTest because TinyUSB submodule is not initialized in the SDK")
endif()