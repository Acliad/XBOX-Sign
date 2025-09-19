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
#include "animation_transistion.h"
#include "led_transition_lut.h"

void animation_transition_init(animation_transition_t *self) {
    self->base.animation_add_led = animation_base_add_led;
    self->base.animation_update = animation_transition_update;
    self->base._head = NULL;

    self->_target_brightness = 0;
    self->_transition_start_brightness = 0;
    self->_transition_lut = LED_LUT_S_CURVE;
    self->_transition_start_ms = 0;
    self->_transition_time_ms = 0;
}

void animation_transition_set(animation_transition_t *self, uint32_t target_brightness, uint32_t transition_time_ms) {
    self->_target_brightness = target_brightness;
    self->_transition_time_ms = transition_time_ms;
    self->_transition_start_ms = 0;
}

animation_status_t animation_transition_update(animation_base_t *self) {
    animation_transition_t *self_transistion = (animation_transition_t *)self;
}
