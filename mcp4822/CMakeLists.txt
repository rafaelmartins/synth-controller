add_library(mcp4822 INTERFACE)

target_sources(mcp4822 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mcp4822.c
)

target_include_directories(mcp4822 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(mcp4822 INTERFACE hardware_spi hardware_gpio)
