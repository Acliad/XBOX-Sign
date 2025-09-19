# This CMakeLists.txt is a mess because I'm learning how all this works...
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# point to the submodule
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/third_party/pico-sdk)
include(third_party/pico-sdk/pico_sdk_init.cmake)

project(xbox_sign C CXX ASM)

# initialize
pico_sdk_init()


add_library(xbox_sign_headers INTERFACE)
target_include_directories(xbox_sign_headers INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(led STATIC
    src/led.c
)
target_link_libraries(led PUBLIC 
    xbox_sign_headers
    PRIVATE
    pico_stdlib
    hardware_pwm
)

add_executable(xbox_sign
    src/main.c
)

pico_enable_stdio_usb(xbox_sign 1)
pico_enable_stdio_uart(xbox_sign 0)

# link pico libs
target_link_libraries(xbox_sign 
    pico_stdlib 
    hardware_gpio 
    led
)

# uf2/hex outputs
pico_add_extra_outputs(xbox_sign)