if (TARGET tinyusb_device)
    pico_sdk_init()

    add_executable(pico_rgb2hdmi_app
        main.c
    )

    target_link_libraries(pico_rgb2hdmi_app
        pico_stdlib
        pico_multicore
        libdvi)
        
    pico_enable_stdio_usb(pico_rgb2hdmi_app 1)
    pico_enable_stdio_uart(pico_rgb2hdmi_app 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building pico_rgb2hdmi_app because TinyUSB submodule is not initialized in the SDK")
endif()