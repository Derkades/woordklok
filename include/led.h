#ifndef LED_H
#define LED_H

#include <list>
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <stdint.h>

#include "effects.h"
#include "config.h"
#include "log.h"
#include "effects.h"

// number of leds per letter
#define LETTER_LEDS 2

#define NUM_ROWS 10
#define NUM_COLS 11
#define NUM_LETTERS NUM_ROWS*NUM_COLS
#define NUM_LEDS NUM_LETTERS*LETTER_LEDS

typedef uint16_t *word_t;

enum DisplayState {
    STATE_BLANK,
    STATE_FADE_OUT,
    STATE_DISPLAY_TIME,
};

extern void led_setup();

extern void led_loop(bool on, uint8_t hue, uint8_t saturation, uint8_t brightness, LedEffect effect);

extern void startup_animation();

#endif // LED_H
