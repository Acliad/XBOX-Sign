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

add_library(animation_base STATIC
    src/animation_base.c
)
target_link_libraries(animation_base PUBLIC 
    xbox_sign_headers
    PRIVATE
    pico_stdlib
)

add_library(animation_transition STATIC
    src/animation_transition.c
)
target_link_libraries(animation_transition PUBLIC 
    xbox_sign_headers
    PRIVATE
    pico_stdlib
    animation_base
)

add_library(animation_breathe STATIC
    src/animation_breathe.c
)
target_link_libraries(animation_breathe PUBLIC 
    xbox_sign_headers
    m
    PRIVATE
    pico_stdlib
    animation_base
)

add_library(animation_sweep STATIC
    src/animation_sweep.c
)
target_link_libraries(animation_sweep PUBLIC 
    xbox_sign_headers
    PRIVATE
    pico_stdlib
    animation_base
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
    animation_transition
    animation_breathe
    animation_sweep
)

# uf2/hex outputs
pico_add_extra_outputs(xbox_sign)