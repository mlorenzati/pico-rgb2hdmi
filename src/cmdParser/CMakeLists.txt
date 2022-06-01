add_library(cmdParser INTERFACE)

target_sources(cmdParser INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/cmdParser.c
    ${CMAKE_CURRENT_LIST_DIR}/cmdParser.h
)

target_include_directories(cmdParser INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(cmdParser INTERFACE
    pico_base_headers
)
