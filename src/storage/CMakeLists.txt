add_library(storage INTERFACE)

target_sources(storage INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/storage.c
    ${CMAKE_CURRENT_LIST_DIR}/storage.h
)

target_include_directories(storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(storage INTERFACE
    pico_base_headers
    hardware_flash
)
