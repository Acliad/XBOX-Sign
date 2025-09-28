/**
 * @file button.c
 * @author Isaac Rex (@Acliad)
 * @brief Button handling library implementation
 * @version 0.1
 * @date 2025-09-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <pico/stdlib.h>
#include "button.h"

static inline bool sample_pressed(const button_t *b) {
    const bool level_high = gpio_get(b->_pin);
    return b->_active_low ? !level_high : level_high;
}

void button_init(button_t *button, uint32_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin); // Default to active-low
    button->_pin         = pin;
    button->_active_low  = true;

    button->_time_ms      = 0u;
    button->_debounce_ms   = BUTTON_DEFAULT_DEBOUNCE_MS;
    button->_long_press_ms = BUTTON_DEFAULT_LONG_PRESS_MS;

    button->_raw_state    = sample_pressed(button);
    button->_state        = button->_raw_state ? BUTTON_PRESSED : BUTTON_RELEASED;
    button->_last_edge_ms = button->_time_ms;

    button->_press_start_ms = (button->_state == BUTTON_PRESSED) ? button->_time_ms : 0u;
    button->_long_fired     = false;

    button->_pending_events = 0u;
}

void button_set_active_low(button_t *button, bool active_low) {
    button->_active_low = active_low;
    if (active_low) {
        gpio_pull_up(button->_pin);
    } else {
        gpio_pull_down(button->_pin);
    }

    // Re-baseline states to current reading to avoid spurious events
    button->_raw_state    = sample_pressed(button);
    button->_state        = button->_raw_state ? BUTTON_PRESSED : BUTTON_RELEASED;
    button->_last_edge_ms = button->_time_ms;

    button->_press_start_ms = (button->_state == BUTTON_PRESSED) ? button->_time_ms : 0u;
    button->_long_fired     = false;
    button->_pending_events = 0u;
}

void button_set_debounce(button_t *button, uint32_t debounce_ms) {
    button->_debounce_ms = debounce_ms;
}

void button_set_long_press_time(button_t *button, uint32_t long_press_ms) {
    button->_long_press_ms = long_press_ms;
}

void button_update(button_t *button, uint32_t dt_ms) {
    button->_time_ms += dt_ms;

    // 1) Track raw edges (logical pressed after polarity)
    const bool raw_now = sample_pressed(button);
    if (raw_now != button->_raw_state) {
        button->_raw_state    = raw_now;
        button->_last_edge_ms = button->_time_ms;
    }

    // 2) Debounce: adopt raw after it has been stable for >= debounce
    if ((button->_time_ms - button->_last_edge_ms) >= button->_debounce_ms) {
        const button_state_t debounced_state = button->_raw_state ? BUTTON_PRESSED : BUTTON_RELEASED;
        if (debounced_state != button->_state) {
            // Edge detected after debounce window
            button->_state = debounced_state;
            if (debounced_state == BUTTON_PRESSED) {
                button->_press_start_ms = button->_time_ms;
                button->_long_fired     = false;
                button->_pending_events |= BUTTON_EVENT_PRESS;
            } else {
                const uint32_t held_ms = button->_time_ms - button->_press_start_ms;
                if (held_ms >= button->_long_press_ms) {
                    button->_pending_events |= BUTTON_EVENT_RELEASE_LONG;
                } else {
                    button->_pending_events |= BUTTON_EVENT_RELEASE_SHORT;
                }
            }
        }
    }

    // 3) Long-press one-shot while held_ms
    if (button->_state == BUTTON_PRESSED && !button->_long_fired) {
        const uint32_t held_ms = button->_time_ms - button->_press_start_ms;
        if (held_ms >= button->_long_press_ms) {
            button->_pending_events |= BUTTON_EVENT_LONG_PRESS;
            button->_long_fired = true;
        }
    }
}

button_state_t button_get_state(const button_t *button) {
    return button->_state;
}

uint32_t button_poll_events(button_t *button) {
    const uint32_t ev = button->_pending_events;
    button->_pending_events = BUTTON_EVENT_NONE;
    return ev;
}

