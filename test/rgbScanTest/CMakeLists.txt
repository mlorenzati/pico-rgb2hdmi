if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(rgbScanTest
        rgbScanTest.c
        )

    target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(rgbScanTest PRIVATE
        rgbScan
        pico_multicore
        pico_stdlib)
        
    pico_enable_stdio_usb(rgbScanTest 1)
    pico_enable_stdio_uart(rgbScanTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building rgbScan because TinyUSB submodule is not initialized in the SDK")
endif()