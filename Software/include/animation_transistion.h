/**
 * @file animation_transistion.h
 * @author Isaac Rex (@Acliad)
 * @brief Animation to transistion smoothly between two values.
 * @version 0.1
 * @date 2025-09-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #pragma once

 #include "animation_base.h"

typedef struct animation_transition_t {
    animation_base_t base;
    uint32_t         _target_brightness;
    uint32_t         _transition_start_brightness;
    uint32_t         _transition_start_ms;
    uint32_t         _transition_time_ms;
    const float     *_transition_lut;
} animation_transition_t;

void animation_transition_init(animation_transition_t *self);
void animation_transition_set(animation_transition_t *self, uint32_t target_brightness, uint32_t transition_time_ms);
animation_status_t animation_transition_update(animation_base_t *self);