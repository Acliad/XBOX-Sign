#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "led.h"

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
    const uint32_t TIMEOUT_MS = 10;
    char input_buff[256];
    led_t led_xl;

    // Initialization
    stdio_init_all();
    led_init(&led_xl, XL_PIN);

    // Let's GOOOO
    sleep_ms(2000); // Wait for USB serial to initialize
    printf("Hello, Xbox Sign!\n"); // Probably won't see this as USB serial is slow to init
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
        led_set_brightness(&led_xl, LED_MAX_BRIGHTNESS, 1000);
        while (led_xl.is_transitioning)
        {
            led_update(&led_xl);
        }
        led_set_brightness(&led_xl, 0, 1000);
        while (led_xl.is_transitioning) {
            led_update(&led_xl);
        }
    }
        // sleep_ms(1000);
    return 0;
}