/**
 * @file animation_base.c
 * @author Isaac Rex (@Acliad)
 * @brief Animation base implementation
 * @version 0.1
 * @date 2025-09-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "animation_base.h"
 #include <stdlib.h>

void animation_base_add_led_node(animation_base_t *self, led_node_t *node) {
    if (!(self && node)) {
        return;
    }
    
    // Set the new node as head if we don't already have one
    if (self->_head == NULL) {
        self->_head = node;
        return;
    }

    // Get the last node in the list and append the new one
    led_node_t *current = self->_head;
    while (current && current->next) {
        current = current->next;
    }
    current->next = node;
    self->_num_leds++;
}

void animation_base_add_led(animation_base_t *self, led_t *led) {
    if (self && led) {
        led_node_t *new_node = (led_node_t *)malloc(sizeof(led_node_t));
        if (new_node) {
            new_node->led = led;
            new_node->next = NULL;
            animation_base_add_led_node(self, new_node);
        }
    }
}

void animation_start(animation_base_t *self) {
    if (self) {
        self->status = ANIMATION_STATUS_RUNNING;
    }
}

void animation_stop(animation_base_t *self) {
    if (self) {
        self->status = ANIMATION_STATUS_STOPPED;
    }
}

led_node_t *animation_base_get_led_node(animation_base_t *self, uint32_t index) {
    if (self == NULL) {
        return NULL;
    }

    led_node_t *current_node = self->_head;
    uint32_t current_index = 0;
    while (current_index < index)
    {
        if (current_node == NULL) {
            return NULL;
        }
        current_node = current_node->next;
        current_index++;
    }

    return current_node;
}
