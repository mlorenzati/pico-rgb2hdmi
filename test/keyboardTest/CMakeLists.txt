if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(keyboardTest
        keyboardTest.c
        )

    target_link_libraries(keyboardTest PRIVATE
        keyboard
        pico_multicore
        pico_stdlib)
        
    pico_enable_stdio_usb(keyboardTest 1)
    pico_enable_stdio_uart(keyboardTest 0)
    pico_add_extra_outputs(keyboardTest)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building keyboardTest because TinyUSB submodule is not initialized in the SDK")
endif()