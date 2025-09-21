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

bool led_init(led_t* led, uint32_t pin);
void led_set_brightness(led_t* led, float brightness);