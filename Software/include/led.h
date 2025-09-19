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
    uint32_t     _pin;
    uint32_t     current_brightness;
    uint32_t     _target_brightness;
    uint32_t     _transition_start_brightness;
    uint32_t     _transition_start_ms;
    uint32_t     _transition_time_ms;
    uint32_t     _slice_number;
    uint32_t     _channel;
    const float *_transition_lut;
    bool         is_transitioning;
} led_t;

bool led_init(led_t* led, uint32_t pin);
void led_set_brightness(led_t* led, uint32_t brightness, uint32_t transition_time_ms);
void led_update(led_t* led); 