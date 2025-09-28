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

#include "animation_transition.h"
#include "led_transition_lut.h"
#include <pico/stdlib.h>
#include <stdlib.h>

void animation_transition_init(animation_transition_t *self) {
    self->base.animation_add_led = animation_base_add_led;
    self->base.animation_update = animation_transition_update;
    self->base.animation_start = animation_transition_start;
    self->base.animation_stop = animation_stop;
    self->base.status = ANIMATION_STATUS_STOPPED;
    self->base._head = NULL;

    self->_target_brightness = 0;
    self->_transition_lut = LED_LUT_S_CURVE;
    self->_transistion_current_ms = 0;
    self->_transition_time_ms = 0;
}

void animation_transition_set(animation_transition_t *self, float target_brightness, uint32_t transition_time_ms) {
    if (self == NULL) {
        return;
    }
    self->_target_brightness = target_brightness;
    self->_transition_time_ms = transition_time_ms;
    self->_transistion_current_ms = 0;
    if (self->base._head) {
        animation_transition_node_t *current_node = (animation_transition_node_t *)self->base._head;
        while (current_node != NULL) {
            current_node->_transition_start_brightness = self->base._head->led->brightness;
            current_node = (animation_transition_node_t *)current_node->base.next;
        }
    }
}

void animation_transition_start(animation_base_t *self) {
    if (self == NULL) {
        return;
    }
    animation_transition_t *transition = (animation_transition_t *)self;
    transition->_transistion_current_ms = 0;
    if (self->_head) {
        animation_transition_node_t *current_node = (animation_transition_node_t *)self->_head;
        while (current_node != NULL) {
            current_node->_transition_start_brightness = current_node->base.led->brightness;
            current_node = (animation_transition_node_t *)current_node->base.next;
        }
    }
    animation_start(self);
}

void animation_transition_add_led(animation_base_t *self, led_t *led) {
    if (self == NULL || led == NULL) {
        return;
    }
    animation_transition_node_t *new_node = (animation_transition_node_t *)malloc(sizeof(animation_transition_node_t));
    if (new_node) {
        new_node->base.next = NULL;
        new_node->base.led = led;
        new_node->_transition_start_brightness = led->brightness;
        animation_base_add_led_node(self, (led_node_t *)new_node);
    }
}

animation_status_t animation_transition_update(animation_base_t *self_base, uint32_t dt_ms) {
    animation_transition_t *self = (animation_transition_t *)self_base;
    if (self->base._head == NULL || self->base.status != ANIMATION_STATUS_RUNNING || dt_ms == 0) {
        return self->base.status; // Leave early if there's nothing to do
    }
    self->_transistion_current_ms += dt_ms;

    /* We need to traverse the lookup table in transisition_time_ms from the start time, starting at index 0 for
     * transistion_start_ms and ending at index LED_LUT_SIZE - 1 for transistion_start_ms + transition_time_ms.
     * Thus, index of interest is: index = (LED_LUT_SIZE - 1) * (current_time_ms - transistion_start_ms) /
     * transition_time_ms
     */
    uint32_t table_index = 0;
    if (self->_transition_time_ms == 0) {
        self->_transistion_current_ms = LED_LUT_SIZE - 1; // Jump to the end of the table immediately
    } else {
        table_index = (LED_LUT_SIZE - 1) * self->_transistion_current_ms / self->_transition_time_ms;
        if (table_index >= LED_LUT_SIZE - 1) { // Transition complete
            table_index = LED_LUT_SIZE - 1; // Clamp to the end
            self->base.status = ANIMATION_STATUS_STOPPED;
        }
    }

    // Cycle through LEDs and update their brightness
    animation_transition_node_t *current_node = (animation_transition_node_t *)self->base._head;
    while (current_node != NULL) {
        float brightness = 0.0f;
        /* Now we need to scale the brightness delta between target and start so that we smoothly transition from
         * where we are to where we're going.
         */
        brightness = current_node->_transition_start_brightness +
                     (self->_target_brightness - current_node->_transition_start_brightness) *
                         self->_transition_lut[table_index];
        led_set_brightness(current_node->base.led, brightness);
        current_node = (animation_transition_node_t *)current_node->base.next;
    }
    return self->base.status;
}
