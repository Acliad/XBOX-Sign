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

void animation_base_add_led(animation_base_t *self, led_t *led)  {
    if (self && led) {
        led_node_t *new_node = (led_node_t *)malloc(sizeof(led_node_t));
        if (new_node) {
            new_node->led = led;
            new_node->next = NULL;

            // Set the new node as head if we don't already have one
            if (self->_head == NULL) {
                self->_head = new_node;
                return;
            }

            // Get the last node in the list and append the new one
            led_node_t *current = self->_head;
            while (current && current->next) {
                current = current->next;
            }
            current->next = new_node;
        }
    }
}