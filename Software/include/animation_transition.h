/**
 * @file animation_transistion.h
 * @author Isaac Rex (@Acliad)
 * @brief Animation to transistion smoothly between two values.
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #pragma once

 #include "animation_base.h"

typedef struct animation_transition_t {
    animation_base_t base;
    uint32_t         _transistion_current_ms;
    uint32_t         _transition_time_ms;
    float            _target_brightness;
    const float     *_transition_lut;
} animation_transition_t;

typedef struct animation_transition_node {
    led_node_t base;
    float _transition_start_brightness; 
} animation_transition_node_t;

/**
 * @brief Initialize an animation transition object. Must be called before use.
 * 
 * @param self Pointer to the animation_transition_t instance
 */
void animation_transition_init(animation_transition_t *self);

/**
 * @brief Set transition parameters for brightness animation. target_brightness is the brightness to transition to.
 * transition_time_ms is the number of milliseconds to take to go to the transition.
 *
 * @param self Pointer to the animation_transition_t instance
 * @param target_brightness Brightness to transition to (0.0 to 1.0)
 * @param transition_time_ms Duration of the transition in milliseconds
 */
void animation_transition_set(animation_transition_t *self, float target_brightness, uint32_t transition_time_ms);

/**
 * @brief Start the animation transition.
 * 
 * @param self_base Pointer to the animation_base_t instance
 */
void animation_transition_start(animation_base_t *self_base);

/**
 * @brief Add a LED to the animation transition.
 * 
 * @param self_base Pointer to the animation_base_t instance
 * @param led Pointer to the led_t instance
 */
void animation_transition_add_led(animation_base_t *self_base, led_t *led);

/**
 * @brief Update the animation transition. This should be called periodically to update the animation state.
 * 
 * @param self_base Pointer to the animation_base_t instance
 * @param dt_ms Delta time in milliseconds since last update
 * @return animation_status_t Status of the animation after update
 */
animation_status_t animation_transition_update(animation_base_t *self_base, uint32_t dt_ms);