if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(graphicsTest
        graphicsTest.c
        )

    target_compile_options(graphicsTest PRIVATE -Wall)

    target_link_libraries(graphicsTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        graphics
        keyboard
        )

    pico_enable_stdio_usb(graphicsTest 1)
    pico_enable_stdio_uart(graphicsTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building graphicsTest because TinyUSB submodule is not initialized in the SDK")
endif()
