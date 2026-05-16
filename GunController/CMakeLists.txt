cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(GunController C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
 
add_executable(GunController 
gun_controller.c
screendrivers/st7789.c
graphics/renderer.cpp
)

pico_enable_stdio_usb(GunController 1)
pico_enable_stdio_uart(GunController 0)
target_link_libraries(GunController pico_stdlib hardware_spi)
pico_add_extra_outputs(GunController)