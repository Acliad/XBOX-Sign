#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "led.h"
#include "animation_transition.h"

typedef enum {
    SERIAL_READ_OK = 0,
    SERIAL_READ_TIMEOUT,
    SERIAL_READ_OVERFLOW,
    SERIAL_READ_NO_DATA
} serial_read_status_t;

serial_read_status_t get_line(char* buffer, uint32_t size, uint32_t timeout_ms) {
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while((to_ms_since_boot(get_absolute_time()) - start_time) < timeout_ms) {
        if (size <= 1) {
            return SERIAL_READ_OVERFLOW; // Leave space for null terminator
        }
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT && c != PICO_ERROR_NO_DATA) {
            *buffer++ = (char)c;
            if (c == '\n' || c == '\r') {
                *buffer = '\0'; // Null-terminate the string
                return SERIAL_READ_OK;
            }
            // Continue
            size--;
            // Got a character, so reset timeout
            start_time = to_ms_since_boot(get_absolute_time());
        }
    }
    return SERIAL_READ_TIMEOUT;
}

int main() {
    // Variables
    const uint XL_PIN = 21;
    const uint B_PIN = 20;
    const uint O_PIN = 19;
    const uint XR_PIN = 18;
    const uint32_t TIMEOUT_MS = 10;
    char input_buff[256];
    led_t led_xl;
    led_t led_b;
    led_t led_o;
    led_t led_xr;
    animation_transition_t anim_transition = {0};

    // Initialization
    stdio_init_all();
    led_init(&led_xl, XL_PIN);
    led_init(&led_b, B_PIN);
    led_init(&led_o, O_PIN);
    led_init(&led_xr, XR_PIN);
    animation_transition_init(&anim_transition);

    // Setup animation
    anim_transition.base.animation_add_led((animation_base_t*)&anim_transition, &led_xl);
    anim_transition.base.animation_add_led((animation_base_t*)&anim_transition, &led_b);
    anim_transition.base.animation_add_led((animation_base_t*)&anim_transition, &led_o);
    anim_transition.base.animation_add_led((animation_base_t*)&anim_transition, &led_xr);

    // Let's GOOOO
    sleep_ms(2000); // Wait for USB serial to initialize
    printf("Hello, Xbox Sign!\n"); // Probably won't see this as USB serial is slow to init
    uint32_t last_time = to_ms_since_boot(get_absolute_time());
    while (true) {
        // serial_read_status_t status = get_line(input_buff, sizeof(input_buff), TIMEOUT_MS);
        // // Simple testing, no real parsing
        // if (status == SERIAL_READ_OK) {
        //     if (input_buff[0] == '1') {
        //         gpio_put(XL_PIN, 1);
        //     } else if (input_buff[0] == '0') {
        //         gpio_put(XL_PIN, 0);
        //     }
        // }
        animation_transition_set(&anim_transition, 0.5f, 2500);
        while (anim_transition.base.status == ANIMATION_STATUS_RUNNING) {
            uint32_t dt = to_ms_since_boot(get_absolute_time()) - last_time;
            last_time = to_ms_since_boot(get_absolute_time());
            animation_transition_update((animation_base_t*)&anim_transition, dt);
            sleep_ms(10);
        }
        animation_transition_set(&anim_transition, 0.15f, 2500);
        while (anim_transition.base.status == ANIMATION_STATUS_RUNNING) {
            uint32_t dt = to_ms_since_boot(get_absolute_time()) - last_time;
            last_time = to_ms_since_boot(get_absolute_time());
            animation_transition_update((animation_base_t*)&anim_transition, dt);
            sleep_ms(10);
        }
        // sleep_ms(1000);
    }
    return 0;
}