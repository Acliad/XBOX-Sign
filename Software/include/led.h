/**
 * @file led.h
 * @author Isaac Rex (@Acliad)
 * @brief LED control functions
 * @version 0.1
 * @date 2025-09-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LED_MAX_BRIGHTNESS ((1 << 12) - 1) // 12-bit resolution
#define LED_USE_GAMMA_CORRECTION true

typedef struct {
    float        brightness; // Use float for simplicity, efficiency not that important in this application
    uint32_t     _pin;
    uint32_t     _slice_number;
    uint32_t     _channel;
} led_t;

/**
 * @brief Initialize the LED. Must be called before use.
 * 
 * @param led Pointer to the LED instance to initialize
 * @param pin GPIO pin the LED is on
 * @return true 
 * @return false 
 */
void led_init(led_t* led, uint32_t pin);

/**
 * @brief Set the brightness of the LED.
 * 
 * @param led Pointer to the LED instance to set brightness of
 * @param brightness Brightness level from 0.0 to 1.0
 */
void led_set_brightness(led_t* led, float brightness);