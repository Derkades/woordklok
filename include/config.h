#ifndef CONFIG_H
#define CONFIG_H

#include <TZ.h>
#include "effects.h"

// WiFi
#define WIFI_HOSTNAME "woordklok"

#define WIFI_AP_ENABLE
#ifdef WIFI_AP_ENABLE
    // Access point to configure WiFi
    #define WIFI_AP_SSID "Woordklok"
    #define WIFI_AP_PASS "klok7135"
    #define WIFI_AP_TITLE "Woorklok"
    #define WIFI_AP_TIMEOUT 10*60
#else
    // Hardcoded WiFi credentials
    #define WIFI_SSID ""
    #define WIFI_PASS ""
#endif

// MQTT
#define MQTT_ENABLED
#ifdef MQTT_ENABLED
    #define MQTT_HOST IPAddress(10, 0, 1, 1)
    #define MQTT_PORT 1883
#endif

// Initial state (or permanent state, if MQTT is not used)
#define INITIAL_EFFECT EFFECT_SHOWER_COLOR_FADE
#define INITIAL_HUE 192
#define INITIAL_SATURATION 192
#define INITIAL_BRIGHTNESS 255

// Time
#define TIMEZONE TZ_Europe_Amsterdam
#define TIME_OFFSET 0 // Number of seconds, useful to make the clock run in advance so you arrive early

// Startup animation
#define STARTUP_ANIMATION_DELAY 20
#define STARTUP_ANIMATION_COLOR_MOVING 0xFFFFFF
#define STARTUP_ANIMATION_COLOR_TRAIL 0x602000
#define STARTUP_ANIMATION_COLOR_BACKGROUND 0x200000

// Effects
#define RAINBOW_SATURATION 192
#define HUE_SHIFT_DEG 90
#define HUE_SHIFT (HUE_SHIFT_DEG / 360.0f) * 256.0f
#define BACKGROUND_DIM 2 // background brightness reduction factor (>=1)
#define RAIN_SPEED 6 // higher is faster

#endif // CONFIG_H
