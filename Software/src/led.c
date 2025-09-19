/**
 * @file led.cpp
 * @author Isaac Rex (@Acliad)
 * @brief 
 * @version 0.1
 * @date 2025-09-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "led.h"
#include "led_transition_lut.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "led_gamma_correction_table.h"

#define PWM_CLOCK_DIV_INT 3
#define PWM_CLOCK_DIV_FRAC 0

bool led_init(led_t *led, uint32_t pin) {
    if (!led) return false;
    // Initialize the struct before setting up the hardware
    led->_pin = pin;
    led->current_brightness = 0;
    led->_target_brightness = 0;
    led->is_transitioning = false;
    led->_transition_time_ms = 0;
    led->_transition_start_ms = 0;    
    led->_transition_lut = LED_LUT_S_CURVE; // Use the S-curve LUT by default
    uint slice_num = pwm_gpio_to_slice_num(pin);
    led->_slice_number = slice_num;
    uint channel = pwm_gpio_to_channel(pin);
    led->_channel = channel;

    // Setup PWM hardware
    pwm_set_chan_level(slice_num, channel, 0); // Start with LED off. Do this first in case the slice is already running
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_wrap(slice_num, LED_MAX_BRIGHTNESS - 1); // Subtract 1 here because a 100% duty cycle is TOP + 1;
    // 125MHz / ((LED_MAX_BRIGHTNESS+1) * 3) ~ 10.172kHz PWM frequency
    pwm_set_clkdiv_int_frac(slice_num, PWM_CLOCK_DIV_INT, PWM_CLOCK_DIV_FRAC); 
    // Start the PWM
    pwm_set_enabled(slice_num, true);
    return true;
}

void led_set_brightness(led_t *led, uint32_t brightness, uint32_t transition_time_ms) {
    if (!led) return;

    // Clamp brightness to valid range
    if (brightness > LED_MAX_BRIGHTNESS) { 
        brightness = LED_MAX_BRIGHTNESS;
    }

    led->_target_brightness = brightness;

    // TODO: Handle transitions over time
    led->_target_brightness = brightness;
    led->_transition_start_brightness = led->current_brightness;
    led->_transition_start_ms = to_ms_since_boot(get_absolute_time());
    led->is_transitioning = true;
    led->_transition_time_ms = transition_time_ms;
}

// NOTE: Does not update any state variables
static void _led_set_pwm(led_t *led, uint32_t brightness) {
    if (!led) return;

    // Clamp brightness to valid range
    if (brightness > LED_MAX_BRIGHTNESS) { 
        brightness = LED_MAX_BRIGHTNESS;
    }

    // TODO: Make gamma correction a seperate function?
#if LED_USE_GAMMA_CORRECTION
    assert(brightness < LED_GAMMA_TABLE_SIZE);
    brightness = LED_GAMMA_CORRECTION_TABLE[brightness];
#endif
    pwm_set_chan_level(led->_slice_number, led->_channel, brightness);
}

void led_update(led_t *led) {
    /* We need to traverse the lookup table in transisition_time_ms from the start time, starting at index 0 for 
     * transistion_start_ms and ending at index LED_LUT_SIZE - 1 for transistion_start_ms + transition_time_ms. Thus, 
     * index of interest is:
     *     index = (LED_LUT_SIZE - 1) * (current_time_ms - transistion_start_ms) / transition_time_ms
     */
    if (!led || !led->is_transitioning) return; // Leave early if there's nothing to do

    // If no transition time is specified, set the brightness immediately
    if (led->_transition_time_ms == 0) {
        _led_set_pwm(led, led->_target_brightness);
        return;
    }

    uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed_time_ms = current_time_ms - led->_transition_start_ms;

    uint32_t table_index = (LED_LUT_SIZE - 1) * elapsed_time_ms / led->_transition_time_ms;
    if (table_index >= LED_LUT_SIZE) { // Transition complete
        table_index = LED_LUT_SIZE - 1;
        led->is_transitioning = false;
    }

    /* Now we need to scale the brightness delta between target and start so that we smoothly transition from where we
     * are to where we're going
     * NOTE: We have to cast to an int here to deal with the negative delta case (dimming). If the max brightness is 
     * ever changed to be 32 bits, this will need to be int64_t. It might also make more sense to just use float for the
     * brightness values...
     */
    led->current_brightness =
        led->_transition_start_brightness +
        ((int32_t)led->_target_brightness - (int32_t)led->_transition_start_brightness) * led->_transition_lut[table_index];
    _led_set_pwm(led, led->current_brightness);
}
