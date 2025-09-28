/**
 * @file config.h
 * @author Isaac Rex (@Acliad)
 * @brief Tiny FLASH based config storage for persisting settings across power cycles. 
 * @note  Does NOT use wear leveling!
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t magic;                 // 'CFG1'
    uint32_t version;               // structure version
    uint32_t crc32;                 // CRC of payload (below)
    uint32_t last_animation_index;  // example value to persist
} app_config_t;

bool config_load(app_config_t* out);              // returns true if loaded/valid
bool config_save(const app_config_t* in);         // returns true on success
void config_set_defaults(app_config_t* cfg);      // set reasonable defaults