#include <list>

#include <Arduino.h>
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <ArduinoOTA.h>
#include <Timezone.h>
#include <ArduinoJson.h>

#include "led_positions.h"
#include "secrets.h"
#include "effects.h"

#define MQTT_TOPIC_BASE            "woordklok"
#define MQTT_TOPIC_LOG             MQTT_TOPIC_BASE "/" "log"
#define MQTT_TOPIC_HA              MQTT_TOPIC_BASE "/" "ha"
#define MQTT_TOPIC_HA_AVAILABILITY MQTT_TOPIC_HA   "/" "availability"
#define MQTT_TOPIC_HA_STATE        MQTT_TOPIC_HA   "/" "state"
#define MQTT_TOPIC_HA_COMMAND      MQTT_TOPIC_HA   "/" "command"

#define MQTT_QOS_AT_MOST_ONCE 0
#define MQTT_QOS_AT_LEAST_ONCE 1
#define MQTT_QOS_EXACTLY_ONCE 2

#define RAINBOW_SATURATION 192

#define STARTUP_ANIMATION_DELAY 20
#define STARTUP_ANIMATION_COLOR_MOVING 0xFFFFFF
#define STARTUP_ANIMATION_COLOR_TRAIL 0x602000
#define STARTUP_ANIMATION_COLOR_BACKGROUND 0x200000

#define NTP_UPDATE_INTERVAL 3600*1000

#define HUE_SHIFT_DEG 90
#define HUE_SHIFT (HUE_SHIFT_DEG / 360.0f) * 256.0f

#define INITIAL_EFFECT EFFECT_SHOWER_COLOR_FADE

const TimeChangeRule SUMMER = {"CEST", Last, Sun, Mar, 2, 120};
const TimeChangeRule WINTER = {"CET ", Last, Sun, Oct, 3, 60};
Timezone localTimezone(SUMMER, WINTER);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, NTP_UPDATE_INTERVAL);

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

enum DisplayState {
    STATE_BOOT,
    STATE_FADE_OUT,
    STATE_DRAW_NEW,
    STATE_DISPLAY_TIME,
};

CRGB leds[222];

short currentTimeMagic = -1;

std::list<word_t> current_words = {};
unsigned short current_words_letter_count = 0;

unsigned short tick = 0;
short led_brightness = 255; // TODO use FastLED scale
uint16_t letter_display_limit = UINT16_MAX;

DisplayState display_state = STATE_BOOT;


// Home Assistant light settings
LedEffect ha_effect         = EFFECT_STATIC;  // Must initially be STATIC for boot effects. Later changed to INITIAL_EFFECT.
bool  ha_state              = true;
short ha_hue                = 192;
short ha_saturation         = 192;
short ha_brightness         = 255;
bool  ha_state_need_publish = true;

void setLetterColor(uint16_t letterPos, CRGB &rgb) {
    for (uint8_t j = 0; j < LETTER_LEDS; j++) {
        leds[letterPos*LETTER_LEDS+j] = rgb;
    }
}

void setLetterColor(uint16_t letterPos, uint32_t rgb) {
    for (uint8_t j = 0; j < LETTER_LEDS; j++) {
        leds[letterPos*LETTER_LEDS+j] = rgb;
    }
}

void log(const String &msg) {
    const char *c_str = msg.c_str();
    if (mqttClient.connected()) {
        mqttClient.publish(MQTT_TOPIC_LOG, MQTT_QOS_AT_LEAST_ONCE, false, c_str);
    }
}

word_t hourWord(const int &hour) {
    int mod = hour % 12;
    return HOURS[(mod == 0 ? 12 : mod) - 1];
}

int getTimeMagic() {
    int minute = timeClient.getMinutes();
    return (int) roundf(minute / 5.0f);
}

void clearWords() {
    current_words.clear();
    current_words_letter_count = 0;
}

void writeWord(word_t word) {
    current_words.push_back(word);
    current_words_letter_count += word[0];
}

void writeTimeToWords() {
    int hour = timeClient.getHours();

    clearWords();

    writeWord(HET);
    writeWord(IS);

    switch(currentTimeMagic) {
        case 0:
            log(String(hour) + " uur");
            writeWord(hourWord(hour));
            writeWord(UUR);
            break;
        case 1:
            log("Vijf over " + String(hour));
            writeWord(VIJF);
            writeWord(hour > 6 ? OVER_2 : OVER);
            writeWord(hourWord(hour));
            break;
        case 2:
            log("Tien over " + String(hour));
            writeWord(TIEN);
            writeWord(hour > 6 ? OVER_2 : OVER);
            writeWord(hourWord(hour));
            break;
        case 3:
            log("Kwart over " + String(hour));
            writeWord(KWART);
            writeWord(OVER_2);
            writeWord(hourWord(hour));
            break;
        case 4:
            log("Tien voor half " + String(hour + 1));
            writeWord(TIEN);
            writeWord(VOOR);
            writeWord(HALF);
            writeWord(hourWord(hour + 1));
            break;
        case 5:
            log("Vijf voor half " + String(hour + 1));
            writeWord(VIJF);
            writeWord(VOOR);
            writeWord(HALF);
            writeWord(hourWord(hour + 1));
            break;
        case 6:
            log("Half " + String(hour + 1));
            writeWord(HALF);
            writeWord(hourWord(hour + 1));
            break;
        case 7:
            log("5 over half " + String(hour + 1));
            writeWord(VIJF);
            writeWord(OVER);
            writeWord(HALF);
            writeWord(hourWord(hour + 1));
            break;
        case 8:
            log("10 over half " + String(hour + 1));
            writeWord(TIEN);
            writeWord(OVER);
            writeWord(HALF);
            writeWord(hourWord(hour + 1));
            break;
        case 9:
            log("Kwart voor " + String(hour + 1));
            writeWord(KWART);
            writeWord(VOOR_2);
            writeWord(hourWord(hour + 1));
            break;
        case 10:
            log("Tien voor " + String(hour + 1));
            writeWord(TIEN);
            writeWord(hour > 6 ? VOOR_2 : VOOR);
            writeWord(hourWord(hour + 1));
            break;
        case 11:
            log("Vijf voor " + String(hour + 1));
            writeWord(VIJF);
            writeWord(hour > 6 ? VOOR_2 : VOOR);
            writeWord(hourWord(hour + 1));
            break;
        case 12:
            log(String(hour+1) + " uur");
            writeWord(hourWord(hour+1));
            writeWord(UUR);
            break;
        default:
            log("Error, unknown magic: " + String(currentTimeMagic));
            break;
    }
}

void connectToWifi() {
    WiFi.setHostname("woordklok");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void connectToMqtt() {
    mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
    connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
    log("MQTT connected");
    mqttClient.subscribe(MQTT_TOPIC_HA_COMMAND, MQTT_QOS_AT_LEAST_ONCE);
    ArduinoOTA.begin(false);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    if (WiFi.isConnected()) {
        mqttReconnectTimer.once(2, connectToMqtt);
    }
}

void onMqttMessage(char* topic, char* payload_unsafe, AsyncMqttClientMessageProperties properties,
            size_t len, size_t index, size_t total) {
    // Create a copy that is null-terminated so it can be safely used by other functions
    char payload[len + 1];
    memcpy(payload, payload_unsafe, len);
    payload[len] = '\0';

    // log("Received mqtt message topic='" + String(topic) + "' payload='" + String(payload) + "'");

    if (strcmp(topic, MQTT_TOPIC_HA_COMMAND) == 0) {
        StaticJsonDocument<512> doc;
        deserializeJson(doc, payload);
        if (doc.containsKey("state")) {
            const char *state = doc["state"];
            if (strcmp(state, "ON") == 0) {
                ha_state = true;
            } else if (strcmp(state, "OFF") == 0) {
                ha_state = false;
            } else {
                log("Ignoring unsupported state: " + String(state));
            }
        }

        if (doc.containsKey("color")) {
            JsonObject color = doc["color"];
            if (color.containsKey("h") && color.containsKey("s")) {
                float h = color["h"];
                float s = color["s"];
                ha_hue        = min((int) (h / 360.0f * 256.0f), 255);
                ha_saturation = min((int) (s / 100.0f * 256.0f), 255);
                // log("New hue=" + String(ha_hue) + " sat=" + String(ha_saturation));
            } else {
                log("Color info is missing h/s property");
            }
        }

        if (doc.containsKey("brightness")) {
            ha_brightness = doc["brightness"];
            // log("Changed brightness: " + String(ha_brightness));
        }

        if (doc.containsKey("effect")) {
            const char *effect_str = doc["effect"];
            LedEffect effect;
            if (ledEffectFromString(effect_str, &effect)) {
                ha_effect = effect;
            } else {
                log("Invalid effect: " + String(effect_str));
            }
        }

        ha_state_need_publish = true;

        return;
    }

    log("Unknown topic");
}

void publishState() {
    StaticJsonDocument<JSON_OBJECT_SIZE(5)> doc;
    doc["state"] = ha_state ? "ON" : "OFF";
    doc["effect"] = ledEffectToString(ha_effect);
    doc["brightness"] = ha_brightness;
    JsonObject color = doc.createNestedObject("color");
    color["h"] = min(360.0f, max(0.0f, ha_hue / 256.0f * 360.0f));
    color["s"] = min(100.0f, max(0.0f, ha_saturation / 256.0f * 100.0f));

    char buf[128];
    if (measureJson(doc) > 128) {
        log("json too large for buffer");
        return;
    }
    serializeJson(doc, buf);
    mqttClient.publish(MQTT_TOPIC_HA_STATE, MQTT_QOS_AT_MOST_ONCE, false, buf);
}

void setupOta() {
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        log("OTA: Start updating " + type);

        for (uint16_t i = 0; i < NUM_LEDS; i++) {
            leds[i] = 0;
        }
        FastLED.show();
    });
    ArduinoOTA.onEnd([]() {
        log("OTA: End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        for (uint16_t letter = 0; letter < NUM_COLS * progress / total; letter++) {
            setLetterColor(letter, 0x00FF00);
        }
        FastLED.show();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        switch(error) {
            case OTA_AUTH_ERROR:
                log("OTA: Auth failed"); break;
            case OTA_BEGIN_ERROR:
                log("OTA: Begin failed"); break;
            case OTA_CONNECT_ERROR:
                log("OTA: Connect failed"); break;
            case OTA_RECEIVE_ERROR:
                log("OTA: Receive failed"); break;
            case OTA_END_ERROR:
                log("OTA: End failed"); break;
            default:
                log("OTA: Unknown error");
        }
    });
}

uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

void writeWordsToLeds() {
    short hue = ha_hue;

    switch (ha_effect) {
        case EFFECT_COLOR_FADE:
        case EFFECT_SHOWER_COLOR_FADE:
            hue = (hue + (tick >> 2)) % 256;
            break;
        case EFFECT_STATIC:
        case EFFECT_RAINBOW:
        case EFFECT_SPARKLE_STATIC:
        case EFFECT_SHOWER:
        case EFFECT_TEST_PATTERN:
            // Use configured hue as-is
            break;
    }

    // Fill background
    for (uint16_t i = 0; i < NUM_LETTERS; i++) {
        CRGB rgb = 0x000000;
        switch(ha_effect) {
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
                if (b <= 2*led_brightness) {
                    uint8_t v = (uint8_t) b;
                    if (v > led_brightness) {
                        v = 2*led_brightness - v;
                    }
                    rgb.setHSV(complementary_hue, ha_saturation, v);
                }
                break;
            }

            case EFFECT_SHOWER:
            case EFFECT_SHOWER_COLOR_FADE:
            {
                const short rain_speed = 6; // higher is faster
                const float dim_constant = .5f; // dimness of background compared to foreground

                short row;
                short col;
                letterToRowCol(i, &row, &col);

                const uint8_t complementary_hue = (uint8_t) (hue + HUE_SHIFT);

                // const short a = 1249;
                const short v = (rain_speed*tick - 50*row + hash(col)) % 13000 % 1200;
                if (v < 256) {
                    // Dim according to led brightness (but slightly dimmer)
                    const float dim = (led_brightness / 256.0f) * dim_constant;
                    const uint8_t v2 = (uint8_t) (v * dim);
                    rgb.setHSV(complementary_hue, ha_saturation, v2);
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

            switch(ha_effect) {
                case EFFECT_TEST_PATTERN:
                case EFFECT_STATIC:
                case EFFECT_COLOR_FADE:
                    rgb.setHSV(hue, ha_saturation, led_brightness);
                    break;
                case EFFECT_RAINBOW:
                    rgb.setHSV(((tick / 4) + 16*letterPos) & 0xFF, RAINBOW_SATURATION, led_brightness);
                    break;
                case EFFECT_SPARKLE_STATIC:
                case EFFECT_SHOWER:
                case EFFECT_SHOWER_COLOR_FADE:
                    CRGB rgb_add;
                    rgb_add.setHSV(hue, ha_saturation, led_brightness);
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

void setupWifi() {
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
}

void setDstOffset() {
    int offsetMinutes = localTimezone.locIsDST(timeClient.getEpochTime()) ? SUMMER.offset : WINTER.offset;
    timeClient.setTimeOffset(offsetMinutes * 60);
}

void startupScanAnimation() {
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

void setup() {
    FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);

    startupScanAnimation();

    setLetterColor(0, 0x00FF00);
    FastLED.show();

    setupWifi();
    while (!mqttClient.connected()) {
        ArduinoOTA.handle();
        delay(10);
    }

    timeClient.update();
    log("Found time: " + timeClient.getFormattedTime());
    setDstOffset();
    log("With DST offset: " + timeClient.getFormattedTime());

    // this doesn't work, why?
    writeWord(HALLO);
    writeWordsToLeds();
    delay(500);

    clearWords();
    writeWord(ROBIN);
    writeWordsToLeds();
    delay(500);

    letter_display_limit = 0;
    display_state = STATE_DRAW_NEW;
    ha_effect = INITIAL_EFFECT;
    currentTimeMagic = getTimeMagic();
    writeTimeToWords();
}

void loop() {
    ArduinoOTA.handle();
    timeClient.update();

    if (tick % 1024 == 0) {
        mqttClient.publish(MQTT_TOPIC_HA_AVAILABILITY, MQTT_QOS_AT_MOST_ONCE, false, "online");
    }

    if (tick % 64 == 0) {
        publishState();
    }

    switch(display_state) {
        case STATE_FADE_OUT: {
            led_brightness -= 4;

            if (led_brightness < 0) {
                led_brightness = 0;

                if (ha_state) {
                    log("Draw new");
                    // Display is now blank, write new time to letters
                    // array then start drawing letters one by one
                    display_state = STATE_DRAW_NEW;
                    letter_display_limit = 0;
                    led_brightness = ha_brightness;
                    currentTimeMagic = getTimeMagic();
                    writeTimeToWords();
                }
            }

            break;
        } case STATE_DRAW_NEW: {
            if (tick % 16 == 0) {
                letter_display_limit++;
                if (letter_display_limit > current_words_letter_count) {
                    // All letters are now displayed, go to constant display state.
                    display_state = STATE_DISPLAY_TIME;
                }
            }

            break;
        } case STATE_DISPLAY_TIME: {
            led_brightness = ha_brightness;

            if (!ha_state ||
                    (tick % 512 == 0 && getTimeMagic() != currentTimeMagic)) {
                log(ha_state ? "Time changed, fade out" : "Turned off, fade out");
                display_state = STATE_FADE_OUT;
            }

            break; }
        default: {
            log("Unknown display state: " + String(display_state));
            delay(500); // prevent log spam
            break;
        }
    }

    writeWordsToLeds();

    tick = (tick + 1) & 0xFFFF;

    delay(10);
}
