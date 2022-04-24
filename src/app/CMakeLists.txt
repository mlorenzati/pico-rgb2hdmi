if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(pico_rgb2hdmi_app
        main.c
        )

    target_compile_options(pico_rgb2hdmi_app PRIVATE -Wall)

    target_compile_definitions(pico_rgb2hdmi_app PRIVATE
        DVI_VERTICAL_REPEAT=2
        DVI_SYMBOLS_PER_WORD=1
        )

    target_include_directories(libdvi INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(pico_rgb2hdmi_app PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        rgbScan)
        

    pico_enable_stdio_usb(pico_rgb2hdmi_app 1)
    pico_enable_stdio_uart(pico_rgb2hdmi_app 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building pico_rgb2hdmi_app because TinyUSB submodule is not initialized in the SDK")
endif()