/**
 * @file led_animation.h
 * @author Isaac Rex (@Acliad)
 * @brief Abstract interface for implementing LED animations.
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <stdint.h>
#include "led.h"

typedef enum {
    ANIMATION_STATUS_STOPPED,
    ANIMATION_STATUS_RUNNING,
} animation_status_t;

typedef struct led_node led_node_t;

struct led_node {
    led_t *led;
    struct led_node *next;
};

typedef struct animation_base animation_base_t;

struct animation_base {
    void (*animation_add_led)(struct animation_base* self, led_t *led);
    animation_status_t (*animation_update)(struct animation_base* self, uint32_t dt_ms);
    led_node_t* _head;
    animation_status_t status;
};

void animation_base_add_led(animation_base_t* self, led_t *led);
