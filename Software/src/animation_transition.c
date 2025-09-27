/**
 * @file animation_transistion.c
 * @author Isaac Rex (@Acliad)
 * @brief Animation to transistion smoothly between two values.
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <pico/stdlib.h>
#include <stdio.h>
#include "animation_transition.h"
#include "led_transition_lut.h"

void animation_transition_init(animation_transition_t *self) {
    self->base.animation_add_led = animation_base_add_led;
    self->base.animation_update = animation_transition_update;
    self->base.animation_start = animation_start;
    self->base.animation_stop = animation_stop;
    self->base.status = ANIMATION_STATUS_STOPPED;
    self->base._head = NULL;

    self->_target_brightness = 0;
    self->_transition_start_brightness = 0;
    self->_transition_lut = LED_LUT_S_CURVE;
    self->_transistion_current_ms = 0;
    self->_transition_time_ms = 0;
}

void animation_transition_set(animation_transition_t *self, float target_brightness, uint32_t transition_time_ms) {
    self->_target_brightness = target_brightness;
    self->_transition_time_ms = transition_time_ms;
    self->base.status = ANIMATION_STATUS_RUNNING;
    self->_transistion_current_ms = 0;
    if (self->base._head) {
         // By definition, all LEDs should be the same brightness
        self->_transition_start_brightness = self->base._head->led->brightness;
    } else {
        self->_transition_start_brightness = 0.0f; // No LEDs, so just start from 0
    }
}

animation_status_t animation_transition_update(animation_base_t *self_base, uint32_t dt_ms) {
    animation_transition_t *self = (animation_transition_t *)self_base;
    if (self->base._head == NULL || self->base.status != ANIMATION_STATUS_RUNNING) {
        return self->base.status; // Leave early if there's nothing to do
    }

    float brightness = 0.0f;

    // If no transition time is specified, set the brightness immediately
    if (self->_transition_time_ms == 0) {
        brightness = (float)self->_target_brightness;
    } else {
        /* We need to traverse the lookup table in transisition_time_ms from the start time, starting at index 0 for
         * transistion_start_ms and ending at index LED_LUT_SIZE - 1 for transistion_start_ms + transition_time_ms.
         * Thus, index of interest is: index = (LED_LUT_SIZE - 1) * (current_time_ms - transistion_start_ms) /
         * transition_time_ms
         */
        self->_transistion_current_ms += dt_ms;

        uint32_t table_index = (LED_LUT_SIZE - 1) * self->_transistion_current_ms / self->_transition_time_ms;
        if (table_index >= LED_LUT_SIZE) { // Transition complete
            table_index = LED_LUT_SIZE - 1;
            self->base.status = ANIMATION_STATUS_STOPPED;
        }

        /* Now we need to scale the brightness delta between target and start so that we smoothly transition from where
         * we are to where we're going NOTE: We have to cast to an int here to deal with the negative delta case
         * (dimming). If the max brightness is ever changed to be 32 bits, this will need to be int64_t. It might also
         * make more sense to just use float for the brightness values...
         */
        brightness =
            self->_transition_start_brightness +
            (self->_target_brightness - self->_transition_start_brightness) * self->_transition_lut[table_index];
    }
    
    // Apply brightness to all LEDs
    for (led_node_t *current_led_node = self->base._head; current_led_node != NULL;
         current_led_node = current_led_node->next) {
            led_set_brightness(current_led_node->led, brightness);
    }
    return self->base.status;
}
