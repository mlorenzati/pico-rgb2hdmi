add_library(autodetection INTERFACE)

target_sources(cmdParser INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/autodetection.c
    ${CMAKE_CURRENT_LIST_DIR}/autodetection.h
)

target_include_directories(autodetection INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(autodetection INTERFACE
    pico_base_headers
    rgbScan
)
