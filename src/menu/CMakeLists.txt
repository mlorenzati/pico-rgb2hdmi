add_library(menu INTERFACE)

target_sources(menu INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/menuGlobals.c
    ${CMAKE_CURRENT_LIST_DIR}/menuGlobals.h
    ${CMAKE_CURRENT_LIST_DIR}/menuCallback.c
    ${CMAKE_CURRENT_LIST_DIR}/menuCallback.h
    ${CMAKE_CURRENT_LIST_DIR}/menu.c
    ${CMAKE_CURRENT_LIST_DIR}/menu.h
)

target_include_directories(menu INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(menu INTERFACE
    pico_base_headers
    graphics
    timer
    keyboard
    gui
    commands
    settings
)
