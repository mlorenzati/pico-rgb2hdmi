if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(menuTest
        menuTest.c
        )

    target_compile_options(menuTest PRIVATE -Wall)

    target_link_libraries(menuTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        graphics
        keyboard
        )

    pico_enable_stdio_usb(menuTest 1)
    pico_enable_stdio_uart(menuTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building menuTest because TinyUSB submodule is not initialized in the SDK")
endif()
