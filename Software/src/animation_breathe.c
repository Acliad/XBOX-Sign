/**
 * @file animation_breathe.c
 * @author Isaac Rex (@Acliad)
 * @brief Animation to breathe LEDs in and out smoothly.
 * @version 0.1
 * @date 2025-09-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "animation_breathe.h"
#include <math.h>
#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>

#define M_2PI (2.0f * M_PI)

void animation_breathe_init(animation_breathe_t *self) {
    self->base.animation_add_led = animation_breathe_add_led;
    self->base.animation_update = animation_breathe_update;
    self->base.animation_start = animation_start;
    self->base.animation_stop = animation_stop;
    self->base.status = ANIMATION_STATUS_STOPPED;
    self->base._head = NULL;
    self->_period_ms = 5000;
    self->_amplitude = 1.0f;
    self->_bias = 0.0f;
    self->_phase = 0.0f;
}

void animation_breathe_set_amplitude(animation_breathe_t *self, float amplitude) {
    self->_amplitude = amplitude;
}

void animation_breathe_set_bias(animation_breathe_t *self, float bias) {
    self->_bias = bias;
}

void animation_breathe_set_period(animation_breathe_t *self, uint32_t period_ms) {
    self->_period_ms = period_ms;
}

void animation_breathe_add_led(animation_base_t *self, led_t *led) {
    animation_breathe_led_node_t *new_node =
        (animation_breathe_led_node_t *)malloc(sizeof(animation_breathe_led_node_t));
    if (new_node) {
        new_node->base.led = led;
        new_node->base.next = NULL;
        new_node->delay_ms = 0;
        animation_base_add_led_node((animation_base_t *)self, &new_node->base);
    }
}

void animation_breathe_set_delay(animation_breathe_t *self, uint32_t index, uint32_t delay_ms) {
    animation_breathe_led_node_t *led_node =
        (animation_breathe_led_node_t *)animation_base_get_led_node((animation_base_t *)self, index);
    if (led_node) {
        led_node->delay_ms = delay_ms;
    }
}

animation_status_t animation_breathe_update(animation_base_t *self_base, uint32_t dt_ms) {
    animation_breathe_t *self = (animation_breathe_t *)self_base;

    float dtheta = M_2PI * (float)(dt_ms) / (float)self->_period_ms;
    self->_phase += dtheta;
    while (self->_phase > M_2PI) {
        self->_phase -= M_2PI;
    }

    animation_breathe_led_node_t *current_node = (animation_breathe_led_node_t *)self->base._head;
    while (current_node != NULL) {
        if (current_node->base.led) {
            float phase_offset = M_2PI * (float)(current_node->delay_ms) / (float)self->_period_ms;
            float brightness = self->_bias + self->_amplitude * (0.5f * (1.0f + sinf(self->_phase + phase_offset)));
            led_set_brightness(current_node->base.led, brightness);
        }
        current_node = (animation_breathe_led_node_t *)current_node->base.next;
    }

    return ANIMATION_STATUS_RUNNING;
}