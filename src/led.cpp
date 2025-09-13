#include "led.h"

// First element denotes array length
uint16_t HET    [] = {3, 109, 108, 107                };
uint16_t IS     [] = {2, 105, 104                     };
uint16_t VIJF   [] = {4, 102, 101, 100, 99            };

uint16_t TIEN   [] = {4, 88, 89, 90, 91               };
uint16_t VOOR   [] = {4, 95, 96, 97, 98               };

uint16_t OVER   [] = {4, 87, 86, 85, 84               };
uint16_t KWART  [] = {5, 81, 80, 79, 78, 77           };

uint16_t HALF   [] = {4, 66, 67, 68, 69               };
uint16_t OVER_2 [] = {4, 73, 74, 75, 76               };

uint16_t VOOR_2 [] = {4, 65, 64, 63, 62               };
uint16_t EEN    [] = {3, 58, 57, 56                   };

uint16_t TWEE   [] = {4, 44, 45, 46, 47               };
uint16_t DRIE   [] = {4, 51, 52, 53, 54               };

uint16_t VIER   [] = {4, 43, 42, 41, 40               };
uint16_t VIJF_2 [] = {4, 39, 38, 37, 36               };
uint16_t ZES    [] = {3, 35, 34, 33                   };

uint16_t ZEVEN  [] = {5, 22, 23, 24, 25, 26           };
uint16_t NEGEN  [] = {5, 28, 29, 30, 31, 32           };

uint16_t ACHT   [] = {4, 21, 20, 19, 18               };
uint16_t TIEN_2 [] = {4, 18, 17, 16, 15               };
uint16_t ELF    [] = {3, 13, 12, 11                   };

uint16_t TWAALF [] = {6, 0, 1, 2, 3, 4, 5             };
uint16_t UUR    [] = {3, 8, 9, 10,                    };

uint16_t ROBIN  [] = {5, 92, 83, 70, 61, 48           };
uint16_t HALLO  [] = {5, 93, 82, 71, 60, 49           };

word_t HOURS[] = {
    EEN, TWEE, DRIE, VIER, VIJF_2, ZES, ZEVEN, ACHT, NEGEN, TIEN_2, ELF, TWAALF
};

static DisplayState display_state = STATE_BOOT;
static CRGB leds[222];
static uint16_t tick;
static uint8_t currentHour = 0;
static uint8_t currentTimeStep = 0;
static std::list<word_t> current_words = {};
static uint16_t current_words_letter_count = 0;
static uint16_t letter_display_limit = UINT16_MAX;

static void letterToRowCol(uint16_t letter, uint8_t *row, uint8_t *col) {
    *row = (NUM_ROWS-1) - letter / NUM_COLS;
    // every other row has swapped direction
    *col = *row % 2 == 0 ? letter % NUM_COLS : (NUM_COLS-1) - (letter % NUM_COLS);
}

static void setLetterColor(uint16_t letterPos, CRGB &rgb) {
    for (uint8_t j = 0; j < LETTER_LEDS; j++) {
        leds[letterPos*LETTER_LEDS+j] = rgb;
    }
}

static void setLetterColor(uint16_t letterPos, uint32_t rgb) {
    for (uint8_t j = 0; j < LETTER_LEDS; j++) {
        leds[letterPos*LETTER_LEDS+j] = rgb;
    }
}

static word_t hourWord(const int &hour) {
    int mod = hour % 12;
    return HOURS[(mod == 0 ? 12 : mod) - 1];
}

static void clearWords() {
    current_words.clear();
    current_words_letter_count = 0;
}

static void writeWord(word_t word) {
    current_words.push_back(word);
    current_words_letter_count += word[0];
}

static void writeTimeToWords() {
    clearWords();

    writeWord(HET);
    writeWord(IS);

    switch(currentTimeStep) {
        case 0:
            log(String(currentHour) + " uur");
            writeWord(hourWord(currentHour));
            writeWord(UUR);
            break;
        case 1:
            log("Vijf over " + String(currentHour));
            writeWord(VIJF);
            writeWord(currentHour > 6 ? OVER_2 : OVER);
            writeWord(hourWord(currentHour));
            break;
        case 2:
            log("Tien over " + String(currentHour));
            writeWord(TIEN);
            writeWord(currentHour > 6 ? OVER_2 : OVER);
            writeWord(hourWord(currentHour));
            break;
        case 3:
            log("Kwart over " + String(currentHour));
            writeWord(KWART);
            writeWord(OVER_2);
            writeWord(hourWord(currentHour));
            break;
        case 4:
            log("Tien voor half " + String(currentHour + 1));
            writeWord(TIEN);
            writeWord(VOOR);
            writeWord(HALF);
            writeWord(hourWord(currentHour + 1));
            break;
        case 5:
            log("Vijf voor half " + String(currentHour + 1));
            writeWord(VIJF);
            writeWord(VOOR);
            writeWord(HALF);
            writeWord(hourWord(currentHour + 1));
            break;
        case 6:
            log("Half " + String(currentHour + 1));
            writeWord(HALF);
            writeWord(hourWord(currentHour + 1));
            break;
        case 7:
            log("5 over half " + String(currentHour + 1));
            writeWord(VIJF);
            writeWord(OVER);
            writeWord(HALF);
            writeWord(hourWord(currentHour + 1));
            break;
        case 8:
            log("10 over half " + String(currentHour + 1));
            writeWord(TIEN);
            writeWord(OVER);
            writeWord(HALF);
            writeWord(hourWord(currentHour + 1));
            break;
        case 9:
            log("Kwart voor " + String(currentHour + 1));
            writeWord(KWART);
            writeWord(VOOR_2);
            writeWord(hourWord(currentHour + 1));
            break;
        case 10:
            log("Tien voor " + String(currentHour + 1));
            writeWord(TIEN);
            writeWord(currentHour > 6 ? VOOR_2 : VOOR);
            writeWord(hourWord(currentHour + 1));
            break;
        case 11:
            log("Vijf voor " + String(currentHour + 1));
            writeWord(VIJF);
            writeWord(currentHour > 6 ? VOOR_2 : VOOR);
            writeWord(hourWord(currentHour + 1));
            break;
        case 12:
            log(String(currentHour + 1) + " uur");
            writeWord(hourWord(currentHour + 1));
            writeWord(UUR);
            break;
        default:
            log("Error, unknown step: " + String(currentTimeStep));
            break;
    }
}

static uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static void writeWordsToLeds(uint8_t hue, uint8_t saturation, LedEffect effect) {
    switch (effect) {
        case EFFECT_COLOR_FADE:
        case EFFECT_SHOWER_COLOR_FADE:
            hue = (hue + (tick >> 2)) & 0xFF;
            break;
        default:
            break;
    }

    // Fill background
    for (uint16_t i = 0; i < NUM_LETTERS; i++) {
        CRGB rgb = 0x000000;
        switch(effect) {
            case EFFECT_STATIC:
            case EFFECT_RAINBOW:
            case EFFECT_COLOR_FADE:
            {
                // Black background
                break;
            }

            case EFFECT_TEST_PATTERN:
            {
                switch(((tick / 64 + i) % 3)) {
                    case 0: rgb.r = 0x40; break;
                    case 1: rgb.g = 0x40; break;
                    case 2: rgb.b = 0x40; break;
                }
                break;
            }

            case EFFECT_SPARKLE_STATIC:
            {
                const uint8_t complementary_hue = (uint8_t) (hue + HUE_SHIFT);

                const short a = 3821;
                const short b = a - (tick + hash(i)) % a;
                if (b <= 2*UINT8_MAX) {
                    uint8_t v = (uint8_t) b;
                    if (v > UINT8_MAX) {
                        v = 2*UINT8_MAX - v;
                    }
                    rgb.setHSV(complementary_hue, saturation, v);
                }
                break;
            }

            case EFFECT_SHOWER:
            case EFFECT_SHOWER_COLOR_FADE:
            {
                uint8_t row;
                uint8_t col;
                letterToRowCol(i, &row, &col);

                const uint8_t complementary_hue = (uint8_t) (hue + HUE_SHIFT);

                const short v = (RAIN_SPEED*tick - 50*row + hash(col)) % 13000 % 1200;
                if (v < 256) {
                    rgb.setHSV(complementary_hue, saturation, (256 - v) / BACKGROUND_DIM);
                }
                break;
            }
        }

        setLetterColor(i, rgb);
    }

    // Fill foreground
    uint16_t drawn_letters = 0;
    for (word_t word : current_words) {
        for (uint16_t i = 1; i < word[0] + 1; i++) {
            if (++drawn_letters > letter_display_limit) {
                goto exit;
            }

            uint16_t letterPos = word[i];

            CRGB rgb = leds[letterPos*LETTER_LEDS];

            switch(effect) {
                case EFFECT_TEST_PATTERN:
                case EFFECT_STATIC:
                case EFFECT_COLOR_FADE:
                    rgb.setHSV(hue, saturation, UINT8_MAX);
                    break;
                case EFFECT_RAINBOW:
                    rgb.setHSV(((tick / 4) + 16*letterPos) & 0xFF, RAINBOW_SATURATION, UINT8_MAX);
                    break;
                case EFFECT_SPARKLE_STATIC:
                case EFFECT_SHOWER:
                case EFFECT_SHOWER_COLOR_FADE:
                    CRGB rgb_add;
                    rgb_add.setHSV(hue, saturation, UINT8_MAX);
                    // Add color to existing background color
                    rgb += rgb_add;
                    break;
            }

            setLetterColor(letterPos, rgb);
        }
    }

    exit:
    FastLED.show();
}

void led_setup() {
    FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);
}

void led_loop(bool on, uint8_t hue, uint8_t saturation, uint8_t brightness, LedEffect effect) {
    // based on getLocalTime, but without blocking
    tm tm;
    time_t t;
    time(&t);
    localtime_r(&t, &tm);

    uint8_t timeStep = (uint8_t) roundf(tm.tm_min / 5.0f);

    switch(display_state) {
        case STATE_BOOT:
            // wait for network time
            if (tm.tm_year > (2020 - 1900)) {
                display_state = STATE_BLANK;
            }
            break;
        case STATE_BLANK:
            if (!on) {
                break; // clock is turned off
            }
            display_state = STATE_DISPLAY_TIME;
            letter_display_limit = 0;
            currentTimeStep = timeStep;
            currentHour = tm.tm_hour;
            writeTimeToWords();
            break;
        case STATE_DISPLAY_TIME:
            FastLED.setBrightness(brightness);

            if (letter_display_limit < NUM_LETTERS && tick % 16 == 0) {
                letter_display_limit++;
            }

            if (!on) {
                log("Turned off, fade out");
                display_state = STATE_FADE_OUT;
            }

            if (timeStep != currentTimeStep) {
                log("Time changed, fade out");
                display_state = STATE_FADE_OUT;
            }

            break;
        case STATE_FADE_OUT: {
            const uint8_t brightness = FastLED.getBrightness();
            if (brightness > 4) {
                FastLED.setBrightness(brightness - 4);
            } else {
                clearWords();
                display_state = STATE_BLANK;
            }

            break;
        } default:
            log("Unknown display state: " + String(display_state));
            delay(500); // prevent log spam
            break;
    }

    writeWordsToLeds(hue, saturation, effect);

    tick = (tick + 1) & 0xFFFF;
}

void startup_animation() {
    for (uint16_t i = 0; i < NUM_LETTERS; i++) {
        setLetterColor(i, STARTUP_ANIMATION_COLOR_BACKGROUND);
    }

    for (uint16_t i = 0; i < NUM_LETTERS; i++) {
        setLetterColor(i, STARTUP_ANIMATION_COLOR_MOVING);
        FastLED.show();
        setLetterColor(i, STARTUP_ANIMATION_COLOR_TRAIL);
        delay(STARTUP_ANIMATION_DELAY);
    }

    // Fade out
    for (int j = 0; j < 40; j++) {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (uint8_t) (leds[i].r * .9f);
            leds[i].g = (uint8_t) (leds[i].g * .9f);
            leds[i].b = (uint8_t) (leds[i].b * .9f);
        }
        FastLED.show();
        delay(STARTUP_ANIMATION_DELAY);
    }
}
