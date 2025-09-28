/**
 * @file config.c
 * @author Isaac Rex (@Acliad)
 * @brief Tiny FLASH based config storage for persisting settings across power cycles. 
 * @note  Does NOT use wear leveling!
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "config.h"
#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h" // for XIP_BASE

#define CFG_MAGIC   0x43464731u   // 'CFG1'
#define CFG_VERSION 1u

// We reserved the last 4 KB by shrinking PICO_FLASH_SIZE_BYTES.
// Our settings live right at that boundary.
#define CFG_SECTOR_SIZE FLASH_SECTOR_SIZE // 4096
#define CFG_PAGE_SIZE   FLASH_PAGE_SIZE   // 256
#ifndef CFG_FLASH_OFFSET
#define CFG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - CFG_SECTOR_SIZE)
#endif

static uint32_t crc32_calc(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            uint32_t m = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & m);
        }
    }
    return ~crc;
}

// CRC over payload only (fields after crc32)
static uint32_t cfg_payload_crc(const app_config_t* cfg) {
    const uint8_t* payload = (const uint8_t*)&cfg->last_animation_index;
    return crc32_calc(payload, sizeof(cfg->last_animation_index));
}

void config_set_defaults(app_config_t* cfg) {
    memset(cfg, 0xFF, sizeof(*cfg)); // make obvious in debugger if unsaved
    cfg->magic = CFG_MAGIC;
    cfg->version = CFG_VERSION;
    cfg->last_animation_index = 0xFF; // Invalid index to force user to set
    cfg->crc32 = cfg_payload_crc(cfg);
}

bool config_load(app_config_t* out) {
    const app_config_t* flash_cfg = (const app_config_t*)(XIP_BASE + CFG_FLASH_OFFSET);
    if (flash_cfg->magic != CFG_MAGIC || flash_cfg->version != CFG_VERSION) {
        return false;
    }
    app_config_t tmp = *flash_cfg;
    if (tmp.crc32 != cfg_payload_crc(&tmp)) {
        return false;
    }
    *out = tmp;
    return true;
}

bool config_save(const app_config_t* in) {
    // Prepare a page buffer in RAM (flash APIs require RAM buffer, size multiple of 256)
    uint8_t page[CFG_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));

    app_config_t tmp = *in;
    tmp.magic = CFG_MAGIC;
    tmp.version = CFG_VERSION;
    tmp.crc32 = cfg_payload_crc(&tmp);

    memcpy(page, &tmp, sizeof(tmp));

    // Erase sector then program first page
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(CFG_FLASH_OFFSET, CFG_SECTOR_SIZE);
    flash_range_program(CFG_FLASH_OFFSET, page, sizeof(page));
    restore_interrupts(ints);

    // Verify
    app_config_t verify;
    if (!config_load(&verify)) return false;
    return verify.last_animation_index == in->last_animation_index;
}