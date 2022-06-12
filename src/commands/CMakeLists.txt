add_library(commands INTERFACE)

target_sources(commands INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/commands.c
    ${CMAKE_CURRENT_LIST_DIR}/commands.h
)

target_include_directories(commands INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(commands INTERFACE
    storage
    security
    graphics
)
