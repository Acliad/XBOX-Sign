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

void led_init(led_t *led, uint32_t pin) {
    if (!led) return;
    // Initialize the struct before setting up the hardware
    led->brightness = 0.0f;
    led->_pin = pin;
    uint slice_num = pwm_gpio_to_slice_num(pin);
    led->_slice_number = slice_num;
    uint channel = pwm_gpio_to_channel(pin);
    led->_channel = channel;

    // Setup PWM hardware
    pwm_set_chan_level(slice_num, channel, 0); // Start with LED off. Do this first in case the slice is already running
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_wrap(slice_num, LED_MAX_BRIGHTNESS - 1); // Subtract 1 here because a 100% duty cycle is TOP + 1;
    // 125MHz / ((LED_MAX_BRIGHTNESS+1) * 3) ~ 10.172kHz PWM frequency for 12-bit resolution
    pwm_set_clkdiv_int_frac(slice_num, PWM_CLOCK_DIV_INT, PWM_CLOCK_DIV_FRAC); 
    // Start the PWM
    pwm_set_enabled(slice_num, true);
}

void led_set_brightness(led_t *led, float brightness) {
    if (!led) return;

    // Clamp brightness to valid range
    if (brightness > 1.0f) { 
        brightness = 1.0f;
    } else if (brightness < 0.0f) {
        brightness = 0.0f;
    }

    uint32_t pwm_comp_value = (uint32_t)(brightness * LED_MAX_BRIGHTNESS);
    // TODO: Make gamma correction an optional separate function?
#if LED_USE_GAMMA_CORRECTION
    assert(pwm_comp_value < LED_GAMMA_TABLE_SIZE);
    pwm_comp_value = LED_GAMMA_CORRECTION_TABLE[pwm_comp_value];
#endif
    pwm_set_chan_level(led->_slice_number, led->_channel, pwm_comp_value);

    led->brightness = brightness;
}