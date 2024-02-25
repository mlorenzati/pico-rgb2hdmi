add_library(rgbScan INTERFACE)

target_sources(rgbScan INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/rgbScan.c
    ${CMAKE_CURRENT_LIST_DIR}/rgbScan.h
    ${CMAKE_CURRENT_LIST_DIR}/nanoSystick.c
    ${CMAKE_CURRENT_LIST_DIR}/nanoSystick.h
    ${CMAKE_CURRENT_LIST_DIR}/wm8213Afe.c
    ${CMAKE_CURRENT_LIST_DIR}/wm8213Afe.h
    ${CMAKE_CURRENT_LIST_DIR}/videoAdjust.c
    ${CMAKE_CURRENT_LIST_DIR}/videoAdjust.h
)

target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(rgbScan INTERFACE
    pico_base_headers
	pico_util
    hardware_spi
    hardware_dma
    hardware_pio
    timer
    color
)

pico_generate_pio_header(rgbScan ${CMAKE_CURRENT_LIST_DIR}/wm8213Afe.pio)