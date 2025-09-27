/**
 * @file animation_breathe.h
 * @author Isaac Rex (@Acliad)
 * @brief Animation to breathe LEDs in and out smoothly.
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #pragma once

 #include "animation_base.h"

typedef struct animation_breathe_t {
    animation_base_t base;
    uint32_t         _period_ms;
    float            _amplitude; // 0 to 1.0f
    float            _bias;    // 0 to 1.0f
    float            _phase;     // 0 to 2*PI
} animation_breathe_t;

typedef struct {
    led_node_t base; // Inherit from led_node_t
    uint32_t delay_ms; // Phase delay of this LED
} animation_breathe_led_node_t;


/**
 * @brief Initialize the breathing animation.
 * 
 * @param self Pointer to the breathing animation instance
 */
void animation_breathe_init(animation_breathe_t *self);


void animation_breathe_set_amplitude(animation_breathe_t *self, float amplitude);
void animation_breathe_set_bias(animation_breathe_t *self, float bias);
void animation_breathe_set_period(animation_breathe_t *self, uint32_t period_ms);

/**
 * @brief Add an LED to the breathing animation.
 * 
 * @param self Pointer to the base animation
 * @param led Pointer to the LED to add
 */
void animation_breathe_add_led(animation_base_t *self, led_t *led);

/**
 * @brief Set the delay (phase delay) for a specific LED in the breathing animation. Index is zero-based and in the
 * order LEDs were added. E.g., the first LED added is index 0, the second is index 1, etc.
 *
 * @param index Index of the LED to set the delay for 
 * @param delay_ms milliseconds of delay to apply to this LED's breathing cycle
 */
void animation_breathe_set_delay(animation_breathe_t *self, uint32_t index, uint32_t delay_ms);

/**
 * @brief Update the breathing animation. Call this periodically with the time elapsed since the last call.
 * 
 * @param self_base Pointer to the base animation instance
 * @param dt_ms Time passed since last update in milliseconds
 * @return animation_status_t 
 */
animation_status_t animation_breathe_update(animation_base_t *self_base, uint32_t dt_ms);