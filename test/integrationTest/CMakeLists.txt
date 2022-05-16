if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(integrationTest
        integrationTest.c
        )

    target_compile_options(integrationTest PRIVATE -Wall)

    target_link_libraries(integrationTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        rgbScan
        overlay
        keyboard
        security
        )

    pico_enable_stdio_usb(integrationTest 1)
    pico_enable_stdio_uart(integrationTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building integrationTest because TinyUSB submodule is not initialized in the SDK")
endif()
