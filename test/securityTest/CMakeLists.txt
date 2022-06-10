if (TARGET tinyusb_device)
    pico_sdk_init()
    add_executable(securityTest
        securityTest.c
        )

    target_link_libraries(securityTest PRIVATE
        security
        storage
        pico_stdlib)
        
    pico_enable_stdio_usb(securityTest 1)
    pico_enable_stdio_uart(securityTest 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(securityTest)

    add_custom_command(TARGET securityTest
        POST_BUILD
        COMMAND echo -n ${RBG2HDMI_HASH_SALT} | cat - ${CMAKE_CURRENT_BINARY_DIR}/securityTest.bin | ${SHA1SUM} -b | cut -d " " -f1 | ${STR2HEX} -r -p > ${CMAKE_CURRENT_BINARY_DIR}/securityTest.hash
        COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/securityTest.bin | cat - ${CMAKE_CURRENT_BINARY_DIR}/securityTest.hash >  ${CMAKE_CURRENT_BINARY_DIR}/securityTest_signed.bin
    )
    
elseif(PICO_ON_DEVICE)
    message(WARNING "not building securityTest because TinyUSB submodule is not initialized in the SDK")
endif()