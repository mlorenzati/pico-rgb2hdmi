add_library(graphics INTERFACE)

target_sources(graphics INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/graphics.c
    ${CMAKE_CURRENT_LIST_DIR}/graphics.h
)

target_include_directories(graphics INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(graphics INTERFACE
    pico_base_headers
)
