if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(spriteTest
        spriteTest.c
        )

    target_compile_options(spriteTest PRIVATE -Wall)
    
    target_compile_definitions(spriteTest PRIVATE
        DVI_VERTICAL_REPEAT=2
    )

    target_link_libraries(spriteTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        libsprite
        keyboard
        )

    pico_add_extra_outputs(spriteTest)

    pico_enable_stdio_usb(spriteTest 1)
    pico_enable_stdio_uart(spriteTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building spriteTest because TinyUSB submodule is not initialized in the SDK")
endif()
