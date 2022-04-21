if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(integrationTest
        integrationTest.c
        )

    target_compile_options(integrationTest PRIVATE -Wall)
    
    target_include_directories(libdvi INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})

    target_link_libraries(integrationTest PRIVATE
        pico_stdlib
        pico_multicore
        pico_util
        libdvi
        rgbScan
        )

    pico_enable_stdio_usb(integrationTest 1)
    pico_enable_stdio_uart(integrationTest 0)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building integrationTest because TinyUSB submodule is not initialized in the SDK")
endif()
