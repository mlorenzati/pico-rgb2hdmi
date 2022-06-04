add_library(system INTERFACE)

target_sources(system INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/system.c
    ${CMAKE_CURRENT_LIST_DIR}/system.h
)

target_include_directories(system INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(system INTERFACE
    pico_base_headers
)
