pico_sdk_init()

add_executable(pico_rgb2hdmi_app
    main.c
)

target_link_libraries(pico_rgb2hdmi_app
    pico_stdlib
    pico_multicore
    libdvi)
    
pico_enable_stdio_uart(pico_rgb2hdmi_app 1)