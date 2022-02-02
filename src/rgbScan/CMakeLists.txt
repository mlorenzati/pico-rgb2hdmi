add_library(rgbScan INTERFACE)

target_sources(rgbScan INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/rgbScan.c
    ${CMAKE_CURRENT_LIST_DIR}/rgbScan.h
    ${CMAKE_CURRENT_LIST_DIR}/nanoSystick.c
    ${CMAKE_CURRENT_LIST_DIR}/nanoSystick.h
    ${CMAKE_CURRENT_LIST_DIR}/fastADC.c
    ${CMAKE_CURRENT_LIST_DIR}/fastADC.h 
)
target_include_directories(rgbScan INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(rgbScan INTERFACE
    pico_base_headers
	pico_util
    hardware_dma
    hardware_pio
)

pico_generate_pio_header(rgbScan ${CMAKE_CURRENT_LIST_DIR}/fastADC.pio)
