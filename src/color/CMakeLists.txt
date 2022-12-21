add_library(color INTERFACE)

target_sources(color INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/color.c
    ${CMAKE_CURRENT_LIST_DIR}/color.h
)

target_include_directories(color INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(color INTERFACE
    pico_base_headers
)
