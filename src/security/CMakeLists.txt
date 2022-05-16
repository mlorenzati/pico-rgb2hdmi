add_library(security INTERFACE)

target_sources(security INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/security.c
    ${CMAKE_CURRENT_LIST_DIR}/security.h
    ${CMAKE_CURRENT_LIST_DIR}/sha1.c
    ${CMAKE_CURRENT_LIST_DIR}/sha1.h
)


target_include_directories(security INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(security INTERFACE
    pico_base_headers
    pico_unique_id
)
