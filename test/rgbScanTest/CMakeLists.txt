if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(rgbScanTest
        rgbScanTest.c
        )

    target_link_libraries(rgbScanTest PRIVATE
        rgbScan
        pico_multicore
        pico_stdlib)
        
    pico_enable_stdio_usb(rgbScanTest 1)
    pico_enable_stdio_uart(rgbScanTest 0)
    pico_add_extra_outputs(rgbScanTest)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building rgbScan because TinyUSB submodule is not initialized in the SDK")
endif()