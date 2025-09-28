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
    button_event_t _pending_events; // bitmask of button_event_t
} button_t;

/**
 * @brief Initialize the button. Must be called before use.
 * 
 * @param button Pointer to the button instance
 * @param pin GPIO pin number for the button
 */
void button_init(button_t *button, uint32_t pin);

/**
 * @brief Set whether the button is active low (pressed = low) or active high (pressed = high).
 * 
 * @param button Pointer to the button instance
 * @param active_low True for active low, false for active high
 */
void button_set_active_low(button_t *button, bool active_low);

/**
 * @brief Set the debounce time for the button. A press event is only registered if the button state is stable for this
 * duration.
 * 
 * @param button Pointer to the button instance
 * @param debounce_ms Debounce time in milliseconds
 */
void button_set_debounce(button_t *button, uint32_t debounce_ms);

/**
 * @brief Set the long press time for the button. A long press event is registered if the button is held for this
 * duration.
 *
 * @param button Pointer to the button instance
 * @param long_press_ms Time in milliseconds to classify as a long press
 */
void button_set_long_press_time(button_t *button, uint32_t long_press_ms);

/**
 * @brief Update the button state. Call this function frequently (e.g., every loop) to handle debouncing and event
 * generation.
 *
 * @param button Pointer to the button instance
 * @param dt_ms Delta time in milliseconds since last call
 */
void button_update(button_t *button, uint32_t dt_ms);

/**
 * @brief Get the current debounced state of the button.
 * 
 * @param button Pointer to the button instance
 * @return button_state_t 
 */
button_state_t button_get_state(const button_t *button);

/**
 * @brief Poll for button events and clear the pending events. Returns a bitmask of button_event_t values.
 * 
 * @param button Pointer to the button instance
 * @return button_event_t Bitmask of events that occurred since the last call to this function
 */
uint32_t button_poll_events(button_t *button);

/**
 * @brief Check if a specific button event is present in the event mask.
 * 
 * @param events Button event bitmask from button_poll_events
 * @param ev Bitmask of event(s) to check for
 * @return true if *any* of the event(s) are present
 * @return false if the event(s) are not present
 */
static inline bool button_has_event(uint32_t events, button_event_t ev) { return (events & ev) != 0; }