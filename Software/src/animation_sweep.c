/**
 * @file animation_sweep.c
 * @author Isaac Rex (@Acliad)
 * @brief Animation implementation to give a "sweeping" effect across LEDs
 * @version 0.1
 * @date 2025-09-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <pico/stdlib.h>
#include <stdlib.h>
#include "animation_sweep.h"

#define ANIMATION_SWEEP_DEFAULT_WINDOW_WIDTH_LEDS 3.0f
#define ANIMATION_SWEEP_DEFAULT_WRAP_WIDTH_LEDS 5.0f
#define ANIMATION_SWEEP_DEFAULT_SPEED_LEDS_PER_S 1.0f
#define ANIMATION_SWEEP_DEFAULT_MIN_BRIGHTNESS 0.0f
#define ANIMATION_SWEEP_DEFAULT_MAX_BRIGHTNESS 1.0f
#define ANIMATION_SWEEP_DEFAULT_RISETIME_MS 2000


void animation_sweep_init(animation_sweep_t *self) {
    self->base.animation_add_led = animation_sweep_add_led;
    self->base.animation_update = animation_sweep_update;
    self->base.animation_start = animation_sweep_start;
    self->base.animation_stop = animation_stop;
    self->base.status = ANIMATION_STATUS_STOPPED;
    self->base._head = NULL;
    self->_wrap_width_leds = ANIMATION_SWEEP_DEFAULT_WRAP_WIDTH_LEDS;
    self->_speed_leds_per_s = ANIMATION_SWEEP_DEFAULT_SPEED_LEDS_PER_S;
    self->_min_brightness = ANIMATION_SWEEP_DEFAULT_MIN_BRIGHTNESS;
    self->_max_brightness = ANIMATION_SWEEP_DEFAULT_MAX_BRIGHTNESS;
    self->_risetime_ms = ANIMATION_SWEEP_DEFAULT_RISETIME_MS;
    self->_position_front_leds = 0.0f;
    self->_position_back_leds = self->_wrap_width_leds - ANIMATION_SWEEP_DEFAULT_WINDOW_WIDTH_LEDS;
    self->_next_led_rise_index = 0;
    self->_next_led_fall_index = (uint32_t)(self->_position_back_leds) + 1;
}

void animation_sweep_set_max_brightness(animation_sweep_t *self, float max_brightness) {
    if (self == NULL) {
        return;
    }
    if (max_brightness < self->_min_brightness) {
        max_brightness = self->_min_brightness;
    }

    self->_max_brightness = max_brightness;
    // Ensure all LED targets are at most this brightness
    animation_sweep_node_t *current_node = (animation_sweep_node_t *)self->base._head;
    while (current_node != NULL) {
        if (current_node->animation._target_brightness > max_brightness) {
            current_node->animation._target_brightness = max_brightness;
        }
        current_node = (animation_sweep_node_t *)current_node->base.next;
    }
}

void animation_sweep_set_min_brightness(animation_sweep_t *self, float min_brightness) {
    if (self == NULL) {
        return;
    }
    if (min_brightness > self->_max_brightness) {
        min_brightness = self->_max_brightness;
    }

    self->_min_brightness = min_brightness;
    // Ensure all LED targets are at least this brightness
    animation_sweep_node_t *current_node = (animation_sweep_node_t *)self->base._head;
    while (current_node != NULL) {
        if (current_node->animation._target_brightness < min_brightness) {
            current_node->animation._target_brightness = min_brightness;
        }
        current_node = (animation_sweep_node_t *)current_node->base.next;
    }
}

void animation_sweep_set_window_width(animation_sweep_t *self, float width_leds) {
    self->_window_width_leds = width_leds;
    self->_position_back_leds = self->_position_front_leds - width_leds;
    while (self->_position_back_leds < 0) {
        self->_position_back_leds += self->_wrap_width_leds;
    }
}

void animation_sweep_set_speed(animation_sweep_t *self, float speed_leds_per_s) {
    self->_speed_leds_per_s = speed_leds_per_s;
}

void animation_sweep_set_wrap_width(animation_sweep_t *self, float wrap_width_leds) {
    self->_wrap_width_leds = wrap_width_leds;
    // Recalibrate the back position, as it might be in the wrong place if the front has wrapped and the back hasn't yet
    self->_position_back_leds = self->_position_front_leds - self->_window_width_leds;
    while (self->_position_back_leds < 0) {
        self->_position_back_leds += self->_wrap_width_leds;
    }
}

void animation_sweep_set_risetime(animation_sweep_t *self, uint32_t risetime_ms) {
    self->_risetime_ms = risetime_ms;
}

void animation_sweep_add_led(animation_base_t *self_base, led_t *led) {
    animation_sweep_t *self = (animation_sweep_t *)self_base;
    if (!(self && led)) {
        return;
    }
    animation_sweep_node_t *new_node = (animation_sweep_node_t *)malloc(sizeof(animation_sweep_node_t));
    if (new_node) {
        new_node->base.next = NULL;
        new_node->base.led = NULL;
        // Setup the transistion animation inside the node
        animation_transition_init(&new_node->animation);
        animation_transition_set(&new_node->animation, self->_min_brightness, self->_risetime_ms);
        new_node->animation.base.animation_add_led((animation_base_t *)&new_node->animation.base, led);

        // Add the node to our linked list
        animation_base_add_led_node((animation_base_t *)self, (led_node_t *)new_node);
    }
}

void animation_sweep_start(animation_base_t *self_base) {
    if (self_base == NULL) {
        return;
    }
    animation_sweep_t *self = (animation_sweep_t *)self_base;
    animation_sweep_node_t *current_node = (animation_sweep_node_t *)self->base._head;
    while (current_node != NULL)
    {
        current_node->animation.base.animation_start((animation_base_t *)&current_node->animation);
        current_node = (animation_sweep_node_t *)current_node->base.next;
    }
    
    animation_start(self_base);
}

animation_status_t animation_sweep_update(animation_base_t *self_base, uint32_t dt_ms) {
    animation_sweep_t *self = (animation_sweep_t *)self_base;
    if (self->base._head == NULL || self->base.status != ANIMATION_STATUS_RUNNING) {
        return self->base.status; 
    }

    float delta_pos = self->_speed_leds_per_s * ((float)dt_ms / 1000.0f);
    self->_position_front_leds += delta_pos;
    self->_position_back_leds += delta_pos;
    while (self->_position_front_leds >= self->_wrap_width_leds) {
        self->_position_front_leds -= self->_wrap_width_leds;
        self->_next_led_rise_index = 0;
    }
    while (self->_position_back_leds >= self->_wrap_width_leds) {
        self->_position_back_leds -= self->_wrap_width_leds;
        self->_next_led_fall_index = 0;
    }

    uint32_t front_pos_index = (uint32_t)(self->_position_front_leds);
    uint32_t back_pos_index = (uint32_t)(self->_position_back_leds);

    animation_sweep_node_t *current_node = (animation_sweep_node_t *)self->base._head;
    uint32_t node_index = 0;
    while (current_node != NULL) {
        if (front_pos_index == node_index && self->_next_led_rise_index <= front_pos_index) {
            // When the wave front "enters" an LED, start a transition to max brightness
            animation_transition_set(&current_node->animation, self->_max_brightness, self->_risetime_ms);
            current_node->animation.base.animation_start(&current_node->animation.base);
            self->_next_led_rise_index++;
        } else if (back_pos_index == node_index && self->_next_led_fall_index <= back_pos_index) {
            // When the wave back "enters" an LED, start a transition to min brightness
            animation_transition_set(&current_node->animation, self->_min_brightness, self->_risetime_ms);
            current_node->animation.base.animation_start(&current_node->animation.base);
            self->_next_led_fall_index++;
        }
        current_node->animation.base.animation_update(&current_node->animation.base, dt_ms);

        current_node = (animation_sweep_node_t *)current_node->base.next;
        node_index++;
    }

    return ANIMATION_STATUS_RUNNING;
}
