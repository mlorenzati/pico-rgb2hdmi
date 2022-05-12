if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(storageTest
        storageTest.c
        )

    target_link_libraries(storageTest PRIVATE
        storage
        hardware_flash
        pico_stdlib)
        
    pico_enable_stdio_usb(storageTest 1)
    pico_enable_stdio_uart(storageTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building storageTest because TinyUSB submodule is not initialized in the SDK")
endif()