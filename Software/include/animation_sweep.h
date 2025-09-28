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


/**
 * @brief Initialize the animation_sweep_t structure. Must be called before use.
 * 
 * @param self Pointer to the animation_sweep_t instance
 */
void animation_sweep_init(animation_sweep_t *self);

/**
 * @brief Set the maximum brightness for the sweep animation.
 * @param self Pointer to the animation_sweep_t instance
 * @param max_brightness Maximum brightness value (0.0 to 1.0)
 */
void animation_sweep_set_max_brightness(animation_sweep_t *self, float max_brightness);

/**
 * @brief Set the minimum brightness for the sweep animation.
 * @param self Pointer to the animation_sweep_t instance
 * @param min_brightness Minimum brightness value (0.0 to 1.0)
 */
void animation_sweep_set_min_brightness(animation_sweep_t *self, float min_brightness);

/**
 * @brief Set the width of the sweep window in LED units. This is how far the LED "wave" extends. Eg. a value of 3.0
 * would mean 3 LEDs are at max brightness at any time.
 * @param self Pointer to the animation_sweep_t instance
 * @param width_leds Width of the sweep window in number of LEDs
 */
void animation_sweep_set_window_width(animation_sweep_t *self, float width_leds);

/**
 * @brief Set the speed of the sweep animation in LEDs per second. So a value of 1.5 would move the wave front 1.5 LEDs
 * in one second.
 * @param self Pointer to the animation_sweep_t instance
 * @param speed_leds_per_s Speed in LEDs per second
 */
void animation_sweep_set_speed(animation_sweep_t *self, float speed_leds_per_s);

/**
 * @brief Set the wrap width for the sweep animation. The animation will wrap around after this many LEDs, and can be
 * more than the number of LEDs in the animation. The wave will continue into these "virtual" LEDs before wrapping
 * around.
 * @param self Pointer to the animation_sweep_t instance
 * @param wrap_width_leds Width at which the animation wraps around in LED units
 */
void animation_sweep_set_wrap_width(animation_sweep_t *self, float wrap_width_leds);

/**
 * @brief Set the rise time for the sweep animation. This is how long it takes for an LED to transition from min to max
 * brightness or vice versa.
 * @param self Pointer to the animation_sweep_t instance
 * @param risetime_ms Rise time in milliseconds
 */
void animation_sweep_set_risetime(animation_sweep_t *self, uint32_t risetime_ms);

/**
 * @brief Add an LED to the sweep animation.
 * @param self Pointer to the animation_base_t instance
 * @param led Pointer to the LED to add to the animation
 */
void animation_sweep_add_led(animation_base_t *self, led_t *led);

/**
 * @brief Start the sweep animation.
 * @param self Pointer to the animation_base_t instance
 */
void animation_sweep_start(animation_base_t *self);

/**
 * @brief Update the sweep animation state. Call this periodically to advance the animation.
 * @param self_base Pointer to the animation_base_t instance
 * @param dt_ms Delta time in milliseconds since last update
 * @return animation_status_t Status of the animation after update
 */
animation_status_t animation_sweep_update(animation_base_t *self_base, uint32_t dt_ms);