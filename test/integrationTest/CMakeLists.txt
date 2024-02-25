if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(integrationTest
        integrationTest.c
        )

    target_compile_options(integrationTest PRIVATE -Wall)

    target_compile_definitions(integrationTest PRIVATE
        DVI_VERTICAL_REPEAT=2
        PICO_TIME_DEFAULT_ALARM_POOL_DISABLED=1
    )

    target_link_libraries(integrationTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        rgbScan
        overlay
        system
        cmdParser
        settings
        commands
        menu
        )

    pico_enable_stdio_usb(integrationTest 0)
    pico_enable_stdio_uart(integrationTest 0)

    pico_add_extra_outputs(integrationTest)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building integrationTest because TinyUSB submodule is not initialized in the SDK")
endif()
