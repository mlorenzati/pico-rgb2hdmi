add_library(keyboard INTERFACE)

target_sources(keyboard INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/keyboard.c
    ${CMAKE_CURRENT_LIST_DIR}/keyboard.h
)

target_include_directories(keyboard INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(keyboard INTERFACE
    pico_base_headers
    timer
)
