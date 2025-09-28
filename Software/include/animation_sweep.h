/**
 * @file animation_sweep.h
 * @author Isaac Rex (@Acliad)
 * @brief Animation implementation to give a "sweeping" effect across LEDs
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once
#include "animation_base.h"
#include "animation_transition.h"

// We'll reuse the led_node_t for our linked list of animation_transition_t. A bit how-ya-doin but it works and let's 
// me reuse the list infrastructure that already exists.
typedef struct animation_sweep_node {
    led_node_t base;
    animation_transition_t animation; 
} animation_sweep_node_t;

typedef struct animation_sweep {
    animation_base_t base;
    float _max_brightness;
    float _min_brightness;
    float _speed_leds_per_s; // Wave front speed in LEDs per second.
    float _wrap_width_leds; // This is how far the wave front travels before wrapping back to the start. Units of LEDs.
    float _window_width_leds; // This is how wide the "on" window is. Units of LEDs.
    float _position_front_leds; // Current position of the "wave front" of the window. Units of LEDs, can be fractional.
    float _position_back_leds; // Current position of the "wave back" of the window. Units of LEDs, can be fractional.
    uint32_t _next_led_rise_index;
    uint32_t _next_led_fall_index;
    uint32_t _risetime_ms;
} animation_sweep_t;


void animation_sweep_init(animation_sweep_t *self);
void animation_sweep_set_max_brightness(animation_sweep_t *self, float max_brightness);
void animation_sweep_set_min_brightness(animation_sweep_t *self, float min_brightness);
void animation_sweep_set_window_width(animation_sweep_t *self, float width_leds);
void animation_sweep_set_speed(animation_sweep_t *self, float speed_leds_per_s);
void animation_sweep_set_wrap_width(animation_sweep_t *self, float wrap_width_leds);
void animation_sweep_set_risetime(animation_sweep_t *self, uint32_t risetime_ms);
void animation_sweep_add_led(animation_base_t *self, led_t *led);
void animation_sweep_start(animation_base_t *self);
animation_status_t animation_sweep_update(animation_base_t *self_base, uint32_t dt_ms);