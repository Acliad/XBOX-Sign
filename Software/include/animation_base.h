/**
 * @file animation_base.h
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
    void (*animation_start)(struct animation_base* self);
    void (*animation_stop)(struct animation_base* self);
    animation_status_t (*animation_update)(struct animation_base* self, uint32_t dt_ms);
    led_node_t* _head;
    animation_status_t status;
    uint32_t _num_leds;
};

/**
 * @brief Append the given node to the end of the linked list of LEDs. The caller must ensure the node is properly
 * allocated and initialized.
 * 
 * @param self base animation instance
 * @param node LED node to add
 */
void animation_base_add_led_node(animation_base_t* self, led_node_t *node);

/**
 * @brief Convenience function to create a new led_node_t and add it to the animation. This will malloc a new node and
 * call animation_base_add_led_node to add it to the linked list.
 *
 * @param self base animation instance
 * @param led LED to add
 */
void animation_base_add_led(animation_base_t* self, led_t *led);

/**
 * @brief Start the animation. Sets status to RUNNING, but does not do anything else. The user should call update to 
 * advance the animation.
 * 
 * @param self Pointer to the animation base instance
 */
void animation_start(animation_base_t* self);

/**
 * @brief Stop the animation. Sets status to STOPPED.
 * 
 * @param self Pointer to the animation base instance
 */
void animation_stop(animation_base_t* self);

/**
 * @brief Get a pointer to the led_node_t at the given index in the linked list. Index is zero-based and in the order
 * LEDs were added. E.g., the first LED added is index 0, the second is index 1, etc.
 * 
 * @param self Pointer to the animation base instance
 * @param index Index of the LED node to retrieve
 * @return led_node_t* 
 */
led_node_t* animation_base_get_led_node(animation_base_t* self, uint32_t index);