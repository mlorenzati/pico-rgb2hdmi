add_library(securemodule INTERFACE)

target_sources(securemodule INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/securemodule.c
    ${CMAKE_CURRENT_LIST_DIR}/securemodule.h
)


target_include_directories(securemodule INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(securemodule INTERFACE
    pico_base_headers
)
