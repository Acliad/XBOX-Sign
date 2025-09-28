/**
 * @file button.h
 * @author Isaac Rex (@Acliad)
 * @brief Simple button handling library with debouncing and short/long press classification.
 * @version 0.1
 * @date 2025-09-26
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef BUTTON_DEFAULT_DEBOUNCE_MS
#define BUTTON_DEFAULT_DEBOUNCE_MS 30u
#endif

#ifndef BUTTON_DEFAULT_LONG_PRESS_MS
#define BUTTON_DEFAULT_LONG_PRESS_MS 1000u
#endif

typedef enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
} button_state_t;

typedef enum {
    BUTTON_EVENT_NONE = 0u,
    BUTTON_EVENT_PRESS = 1u << 0,         // debounced press edge
    BUTTON_EVENT_LONG_PRESS = 1u << 1,    // fired once while held, when held >= long threshold
    BUTTON_EVENT_RELEASE_SHORT = 1u << 2, // release before long threshold
    BUTTON_EVENT_RELEASE_LONG = 1u << 3,  // release after long threshold
} button_event_t;

typedef struct {
    uint8_t _pin;
    bool    _active_low;

    // Debounce and state
    bool           _raw_state;    // last sampled raw (logical pressed/released after polarity)
    button_state_t _state;        // debounced stable state
    uint32_t       _last_edge_ms; // when raw changed (for debounce)
    uint32_t       _debounce_ms;

    // Press tracking
    uint32_t _time_ms;         
    uint32_t _press_start_ms;
    uint32_t _long_press_ms;
    bool     _long_fired;

    // Events
    uint32_t _pending_events; // bitmask of button_event_t
} button_t;

// Init as active-low with pull-up by default
void button_init(button_t *button, uint32_t pin);

// Optional: configure polarity/parameters
void button_set_active_low(button_t *button, bool active_low);
void button_set_debounce(button_t *button, uint32_t debounce_ms);
void button_set_long_press_time(button_t *button, uint32_t long_press_ms);

// Call often (e.g., every loop); handles debouncing and event generation
void button_update(button_t *button, uint32_t dt_ms);

// Read debounced state
button_state_t button_get_state(const button_t *button);

// Fetch and clear pending events bitmask (OR of button_event_t)
uint32_t button_poll_events(button_t *button);

// Utility to test a bitmask
static inline bool button_has_event(uint32_t events, button_event_t ev) { return (events & ev) != 0; }