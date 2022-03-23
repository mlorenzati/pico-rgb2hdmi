if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(dacTest
        dacTest.c
        )

    target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(dacTest PRIVATE
        rgbScan
        pico_multicore
        pico_stdlib)

    pico_enable_stdio_usb(dacTest 1)
    pico_enable_stdio_uart(dacTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building dacTest because TinyUSB submodule is not initialized in the SDK")
endif()