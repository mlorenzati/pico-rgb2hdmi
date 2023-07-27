if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(guiTest
        guiTest.c
        )

    target_compile_options(guiTest PRIVATE -Wall)

    target_compile_definitions(guiTest PRIVATE
    DVI_VERTICAL_REPEAT=2
    )

    target_link_libraries(guiTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        graphics
        gui
        keyboard
        )

    pico_enable_stdio_usb(guiTest 1)
    pico_enable_stdio_uart(guiTest 0)
    pico_add_extra_outputs(guiTest)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building guiTest because TinyUSB submodule is not initialized in the SDK")
endif()
