add_library(settings INTERFACE)

target_sources(settings INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/settings.c
    ${CMAKE_CURRENT_LIST_DIR}/settings.h
)

target_include_directories(settings INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(settings INTERFACE
    pico_base_headers
    storage
)
