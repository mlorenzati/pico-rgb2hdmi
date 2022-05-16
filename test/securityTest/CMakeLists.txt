if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(securityTest
        securityTest.c
        )

    target_link_libraries(securityTest PRIVATE
        security
        pico_stdlib)
        
    pico_enable_stdio_usb(securityTest 1)
    pico_enable_stdio_uart(securityTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building securityTest because TinyUSB submodule is not initialized in the SDK")
endif()