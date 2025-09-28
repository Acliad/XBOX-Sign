#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "led.h"
#include "button.h"
#include "config.h"
#include "pico/bootrom.h"
#include "animation_transition.h"
#include "animation_breathe.h"
#include "animation_sweep.h"

#define arrsizeof(x) (sizeof(x) / sizeof((x)[0]))

// Number of ms to wait after requesting a config save before actually saving. Debounces multiple save requests.
#define CONFIG_SAVE_WAIT_MS (10000)

#define ANIM_OFF_TRANSITION_TIME_MS (500)

#define ANIM_STATIC_TRANSITION_TIME_MS (400)
#define ANIM_STATIC_1_BRIGHTNESS (1.0f)
#define ANIM_STATIC_2_BRIGHTNESS (0.7f)
#define ANIM_STATIC_3_BRIGHTNESS (0.25f)

#define ANIM_BREATHE_PERIOD_MS (10000)
#define ANIM_BREATHE_AMPLITUDE (0.8f)
#define ANIM_BREATHE_BIAS      (0.2f)

#define ANIM_FLOW_PERIOD_MS (15000)
#define ANIM_FLOW_AMPLITUDE (ANIM_BREATHE_AMPLITUDE)
#define ANIM_FLOW_BIAS      (ANIM_BREATHE_BIAS)

#define ANIM_SWEEP_RISETIME_MS       (2500)
#define ANIM_SWEEP_SPEED_LEDS_PER_S  (0.9f)
#define ANIM_SWEEP_WINDOW_WIDTH_LEDS (1.35f)
#define ANIM_SWEEP_WRAP_WIDTH_LEDS   (6.0f)
#define ANIM_SWEEP_MAX_BRIGHTNESS    (1.0f)
#define ANIM_SWEEP_MIN_BRIGHTNESS    (0.2f)



const uint32_t FLOW_LED_OFFSETS_MS[] = {(ANIM_FLOW_PERIOD_MS * 4) / 5, (ANIM_FLOW_PERIOD_MS * 3) / 5,
                                        (ANIM_FLOW_PERIOD_MS * 2) / 5, (ANIM_FLOW_PERIOD_MS * 1) / 5, 0};

typedef enum {
    SERIAL_READ_OK = 0,
    SERIAL_READ_TIMEOUT,
    SERIAL_READ_OVERFLOW,
    SERIAL_READ_NO_DATA
} serial_read_status_t;

typedef enum {
    STATE_OFF = 0,
    STATE_ANIMATING,
    STATE_NEXT_ANIMATION,
    STATE_TURNING_OFF
} xbox_sign_state_t;

void check_program_request(const uint32_t BUTTON_PIN) {
    // Check if the button is held during startup.
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN); // External pull-up on board
    printf(">> Checking for reprogram request...\n");
    if (gpio_get(BUTTON_PIN) == 0) {
        printf(">> Button held during startup, entering programming mode.\n");
        sleep_ms(100); // Allow time for message to send
        reset_usb_boot(0, 0);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(1500); // Wait for USB serial to initialize
    printf("###########################################\n");
    printf("#                                         #\n");
    printf("#    ##   ## ######   #######  ##   ##    #\n");
    printf("#     ## ##  ##   ## ##     ##  ## ##     #\n");
    printf("#      ###   ######  ##     ##   ###      #\n");
    printf("#     ## ##  ##   ## ##     ##  ## ##     #\n");
    printf("#    ##   ## ######   #######  ##   ##    #\n");
    printf("#                                         #\n");
    printf("#             SIGN CONTROLLER             #\n");
    printf("#                 v1.0.0                  #\n");
    printf("#                                         #\n");
    printf("###########################################\n");

    app_config_t app_config;
    const uint32_t BUTTON_PIN = 16;
    const uint32_t SYMBOL_PIN = 22;
    const uint32_t XL_PIN = 21;
    const uint32_t B_PIN = 20;
    const uint32_t O_PIN = 19;
    const uint32_t XR_PIN = 18;
    const uint32_t TIMEOUT_MS = 10;
    xbox_sign_state_t state = STATE_ANIMATING;
    char input_buff[256];
    button_t button;
    led_t led_symbol;
    led_t led_xl;
    led_t led_b;
    led_t led_o;
    led_t led_xr;
    led_t *leds[] = {&led_symbol, &led_xl, &led_b, &led_o, &led_xr};
    animation_transition_t anim_static_1 = {0};
    animation_transition_t anim_static_2 = {0};
    animation_transition_t anim_static_3 = {0};
    animation_breathe_t    anim_breathe = {0};
    animation_breathe_t    anim_flow = {0};
    animation_sweep_t      anim_sweep = {0};
    animation_transition_t anim_turn_off = {0};
    animation_base_t      *animations[] = {
        &anim_static_1.base,
        &anim_static_2.base,
        &anim_static_3.base,
        &anim_breathe.base,
        &anim_flow.base,
        &anim_sweep.base,
        &anim_turn_off.base
    };
    const uint32_t animation_turn_off_index = arrsizeof(animations)-1; 
    uint32_t current_animation_index = 0; // Default value

    // Initialization
    printf("\n> System initializing...\n");

    // Check for reprogram request. A little hacky, but no need to go too hard here.
    // Just check if the button is held during startup.
    check_program_request(BUTTON_PIN);

    button_init(&button, BUTTON_PIN);
    led_init(&led_symbol, SYMBOL_PIN);
    led_init(&led_xl, XL_PIN);
    led_init(&led_b, B_PIN);
    led_init(&led_o, O_PIN);
    led_init(&led_xr, XR_PIN);
    animation_transition_init(&anim_static_1);
    animation_transition_init(&anim_static_2);
    animation_transition_init(&anim_static_3);
    animation_breathe_init(&anim_breathe);
    animation_breathe_init(&anim_flow);
    animation_sweep_init(&anim_sweep);
    animation_transition_init(&anim_turn_off);

    // Load config
    config_set_defaults(&app_config);
    bool config_valid = config_load(&app_config) && (app_config.last_animation_index < animation_turn_off_index);
    if (!config_valid) {
        printf(">> No valid config found, using defaults.\n");
    } else {
        current_animation_index = app_config.last_animation_index;
        printf(">> Config loaded from flash. Last animation index: %u\n", current_animation_index);
    }

    // LED and animation setup
    printf(">> Adding LEDs to animations...\n");
    for (uint32_t i = 0; i < arrsizeof(animations); i++) {
        for (uint32_t j = 0; j < arrsizeof(leds); j++) {
            animations[i]->animation_add_led(animations[i], leds[j]);
        }
    }

    // Setup animation
    printf(">> Configuring animations...\n");
    // Breathe
    animation_breathe_set_amplitude(&anim_breathe, ANIM_BREATHE_AMPLITUDE);
    animation_breathe_set_bias(&anim_breathe, ANIM_BREATHE_BIAS);
    animation_breathe_set_period(&anim_breathe, ANIM_BREATHE_PERIOD_MS);

    // Flow
    animation_breathe_set_amplitude(&anim_flow, ANIM_FLOW_AMPLITUDE);
    animation_breathe_set_bias(&anim_flow, ANIM_FLOW_BIAS);
    animation_breathe_set_period(&anim_flow, ANIM_FLOW_PERIOD_MS);
    _Static_assert(arrsizeof(FLOW_LED_OFFSETS_MS) == arrsizeof(leds));
    for (uint32_t i = 0; i < arrsizeof(FLOW_LED_OFFSETS_MS); i++) {
        animation_breathe_set_delay(&anim_flow, i, FLOW_LED_OFFSETS_MS[i]);
    }
    // Sweep
    animation_sweep_set_max_brightness(&anim_sweep, ANIM_SWEEP_MAX_BRIGHTNESS);
    animation_sweep_set_min_brightness(&anim_sweep, ANIM_SWEEP_MIN_BRIGHTNESS);
    animation_sweep_set_risetime(&anim_sweep, ANIM_SWEEP_RISETIME_MS);
    animation_sweep_set_speed(&anim_sweep, ANIM_SWEEP_SPEED_LEDS_PER_S);
    animation_sweep_set_window_width(&anim_sweep, ANIM_SWEEP_WINDOW_WIDTH_LEDS);
    animation_sweep_set_wrap_width(&anim_sweep, ANIM_SWEEP_WRAP_WIDTH_LEDS);
    // Static 100%
    animation_transition_set(&anim_static_1, ANIM_STATIC_1_BRIGHTNESS, ANIM_STATIC_TRANSITION_TIME_MS);
    // Static 75%
    animation_transition_set(&anim_static_2, ANIM_STATIC_2_BRIGHTNESS, ANIM_STATIC_TRANSITION_TIME_MS);
    // Static 50%
    animation_transition_set(&anim_static_3, ANIM_STATIC_3_BRIGHTNESS, ANIM_STATIC_TRANSITION_TIME_MS);
    // Turn off
    animation_transition_set(&anim_turn_off, 0.0f, ANIM_OFF_TRANSITION_TIME_MS);

    printf("> System initialization complete. Starting...\n");
    animations[current_animation_index]->animation_start(animations[current_animation_index]);
    uint32_t last_loop_time_ms = to_ms_since_boot(get_absolute_time());
    uint32_t last_save_request_time_ms = to_ms_since_boot(get_absolute_time());
    bool save_requested = false;
    while (true) {
        sleep_ms(20);
        uint32_t dt_ms = to_ms_since_boot(get_absolute_time()) - last_loop_time_ms;
        last_loop_time_ms = to_ms_since_boot(get_absolute_time());
        button_update(&button, dt_ms);
        button_event_t button_events = button_poll_events(&button);

        // Handle config
        if (save_requested && (last_loop_time_ms - last_save_request_time_ms) >= CONFIG_SAVE_WAIT_MS) {
            printf("> Saving config to flash...\n");
            app_config.last_animation_index = current_animation_index;
            if (config_save(&app_config)) {
                printf(">> Config saved.\n");
            } else {
                printf(">> [ERROR] Failed to save config!\n");
            }
            save_requested = false;
        }
        switch (state)
        {
        case STATE_OFF:
            if (button_events & (BUTTON_EVENT_RELEASE_SHORT | BUTTON_EVENT_LONG_PRESS)) {
                animations[current_animation_index]->animation_start(animations[current_animation_index]);
                state = STATE_ANIMATING;
                printf("> Turning on...\n");
            }
            break;
        case STATE_ANIMATING:
            if (button_events & BUTTON_EVENT_LONG_PRESS) {
                animations[current_animation_index]->animation_stop(animations[current_animation_index]);
                animations[animation_turn_off_index]->animation_start(animations[animation_turn_off_index]);
                state = STATE_TURNING_OFF;
                printf("> Turning off...\n");
            } else if (button_events & BUTTON_EVENT_RELEASE_SHORT) {
                state = STATE_NEXT_ANIMATION;
            } else {
                animations[current_animation_index]->animation_update(animations[current_animation_index], dt_ms);
            }
            break;

        case STATE_NEXT_ANIMATION:
            animations[current_animation_index]->animation_stop(animations[current_animation_index]);
            current_animation_index = (current_animation_index + 1) % (animation_turn_off_index);
            animations[current_animation_index]->animation_start(animations[current_animation_index]);

            // Setup config and request save
            save_requested = true;
            last_save_request_time_ms = to_ms_since_boot(get_absolute_time());
            state = STATE_ANIMATING;
            printf("> Switched to animation %u\n", current_animation_index);
            break;

        case STATE_TURNING_OFF:
            animations[animation_turn_off_index]->animation_update(animations[animation_turn_off_index], dt_ms);
            if (animations[animation_turn_off_index]->status == ANIMATION_STATUS_STOPPED) {
                state = STATE_OFF;
            }
            break;
        default:
            printf("> [ERROR] In an unknown state!\n");
            break;
        }
    }
    return 0;
}
