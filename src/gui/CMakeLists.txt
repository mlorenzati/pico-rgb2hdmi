add_library(gui INTERFACE)

target_sources(gui INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/gui.c
    ${CMAKE_CURRENT_LIST_DIR}/gui.h
)

target_include_directories(gui INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(gui INTERFACE
    pico_base_headers
    graphics
)
