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

/**
 * @brief Load the application configuration from FLASH into the provided structure.
 * 
 * @param out Pointer to the config instance to load to
 * @return true if the config was loaded successfully
 * @return false if the config was not valid (e.g., bad magic, version, or CRC)
 */
bool config_load(app_config_t* out);

/**
 * @brief Save the application configuration from the provided structure to FLASH.
 * 
 * @param in Pointer to the config instance to save
 * @return true if the instance was saved successfully
 * @return false if the save failed (e.g., FLASH write error)
 */
bool config_save(const app_config_t* in);

/**
 * @brief Set default values for the application configuration.
 * 
 * @param cfg Pointer to the config instance to initialize
 */
void config_set_defaults(app_config_t* cfg);