add_library(overlay INTERFACE)

target_sources(overlay INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/overlay.c
    ${CMAKE_CURRENT_LIST_DIR}/overlay.h
)


target_include_directories(overlay INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(overlay INTERFACE
    pico_base_headers
)
