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

typedef struct {
    led_t *led;
    led_t *next;
} led_node_t;

typedef struct animation_base_t;

typedef struct {
    void (*animation_add_led)(animation_base_t* self, led_t *led);
    animation_status_t (*animation_update)(animation_base_t* self, uint32_t dt_ms);
    led_node_t* _head;
} animation_base_t;

void animation_base_add_led(animation_base_t* self, led_t *led);
