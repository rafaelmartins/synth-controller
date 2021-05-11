cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(synth-controller)

pico_sdk_init()

add_executable(synth-controller
    main.c
    controller.c
    cv-note.c
    usb.c
)

target_link_libraries(synth-controller PRIVATE
    pico_stdlib
    hardware_i2c
    hardware_spi
    tinyusb_device
    mcp4822
    ssd1306
)

target_include_directories(synth-controller PRIVATE
    include
    .
)

target_compile_definitions(tinyusb_device INTERFACE
    TUD_OPT_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

pico_enable_stdio_uart(synth-controller 1)

add_subdirectory(mcp4822)
add_subdirectory(ssd1306)
