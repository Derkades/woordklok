#include "main.h"

#ifdef MQTT_ENABLED
#define MQTT_TOPIC_BASE            "woordklok"
#define MQTT_TOPIC_LOG             MQTT_TOPIC_BASE "/" "log"
#define MQTT_TOPIC_HA              MQTT_TOPIC_BASE "/" "ha"
#define MQTT_TOPIC_HA_AVAILABILITY MQTT_TOPIC_HA   "/" "availability"
#define MQTT_TOPIC_HA_STATE        MQTT_TOPIC_HA   "/" "state"
#define MQTT_TOPIC_HA_COMMAND      MQTT_TOPIC_HA   "/" "command"

#define MQTT_QOS_AT_MOST_ONCE 0
#define MQTT_QOS_AT_LEAST_ONCE 1
#define MQTT_QOS_EXACTLY_ONCE 2

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
bool ha_state_need_publish = true;
#endif

// Home Assistant (MQTT) light settings
bool      ha_on                 = true;
LedEffect ha_effect             = INITIAL_EFFECT;
uint8_t   ha_hue                = INITIAL_HUE;
uint8_t   ha_saturation         = INITIAL_SATURATION;
uint8_t   ha_brightness         = INITIAL_BRIGHTNESS;

void log(const String &msg) {
    #ifdef DEBUG_SERIAL
    Serial.println(msg);
    #endif
    #ifdef MQTT_ENABLED
    const char *c_str = msg.c_str();
    if (mqttClient.connected()) {
        mqttClient.publish(MQTT_TOPIC_LOG, MQTT_QOS_AT_LEAST_ONCE, false, c_str);
    }
    #endif
}

#ifdef MQTT_ENABLED
void connectToMqtt() {
    mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
    log("MQTT connected");
    mqttClient.subscribe(MQTT_TOPIC_HA_COMMAND, MQTT_QOS_AT_LEAST_ONCE);
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

    if (strcmp(topic, MQTT_TOPIC_HA_COMMAND) == 0) {
        StaticJsonDocument<512> doc;
        deserializeJson(doc, payload);
        if (doc.containsKey("state")) {
            const char *state = doc["state"];
            if (strcmp(state, "ON") == 0) {
                ha_on = true;
            } else if (strcmp(state, "OFF") == 0) {
                ha_on = false;
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
    doc["state"] = ha_on ? "ON" : "OFF";
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
#endif

#ifdef MQTT_ENABLED
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
    connectToMqtt();
}
#endif

void setupWifi() {
    #ifdef MQTT_ENABLED
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    static WiFiEventHandler wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    #endif

    #ifdef WIFI_AP_ENABLE
    WiFiManager wm;
    wm.setTitle(WIFI_AP_TITLE);
    wm.setHostname(WIFI_HOSTNAME);
    wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT);
    wm.setConfigPortalTimeout(WIFI_AP_TIMEOUT);
    wm.setConfigPortalBlocking(false);

    if (wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASS)) {
        // connected
        return;
    }

    bool configure_led_state = false;
    while (wm.process() == false) {
        delay(200);
        configure_led_state = !configure_led_state;
        status_led(configure_led_state ? 0xFFFF00 : 0);

        if (millis() > WIFI_AP_TIMEOUT*1000) {
            Serial.println("timeout reached, restarting ESP");
            status_led(0xFF0000);
            delay(5000);
            ESP.restart();
            return;
        }
    }

    Serial.println("configuration complete");
    delay(5000);
    ESP.restart();
    #else
    #ifdef ESP8266
    wifi_station_set_hostname(WIFI_HOSTNAME);
    #else
    WiFi.setHostname(WIFI_HOSTNAME);
    #endif
    WiFi.persistent(false); // avoid unnecessary flash write cycles
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    #endif // WIFI_AP_ENABLE
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
    });
    ArduinoOTA.onEnd([]() {
        log("OTA: End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
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

void setup() {
    #ifdef DEBUG_SERIAL
    Serial.begin(115200);
    #endif

    led_setup();
    #ifdef ESP8266
    configTime(TIMEZONE, "pool.ntp.org");
    #else
    configTime(TIMEZONE, "pool.ntp.org");
    #endif
    startup_animation();
    setupWifi();
    #ifdef ESP8266
    ArduinoOTA.begin(false);
    #else
    ArduinoOTA.begin();
    #endif
}

void loop() {
    ArduinoOTA.handle();

    #ifdef MQTT_ENABLED
    // publish online state every 10 seconds
    static unsigned long ha_last_availability = 0;
    if (millis() - ha_last_availability > 10000 || ha_last_availability > millis()) {
        mqttClient.publish(MQTT_TOPIC_HA_AVAILABILITY, MQTT_QOS_AT_MOST_ONCE, false, "online");
        ha_last_availability = millis();
    }

    if (ha_state_need_publish) {
        publishState();
        ha_state_need_publish = false;
    }
    #endif

    uint8_t brightness = ha_brightness;

    #ifdef LDR_ENABLED
    static uint16_t ldr_avg = 0;
    ldr_avg = (ldr_avg * 7 + (uint16_t) analogRead(LDR_PIN)) / 8;
    uint8_t ldr_brightness = constrain(map(ldr_avg, LDR_INPUT_MIN, LDR_INPUT_MAX, 0, UINT8_MAX), LDR_BRIGHTNESS_MIN, LDR_BRIGHTNESS_MAX);
    // brightness = sqrt16((uint16_t) ldr_brightness * (uint16_t) ha_brightness);
    brightness = ldr_brightness;

    #ifdef LDR_DEBUG
    static unsigned long last_ldr = 0;
    if (millis() - last_ldr > 1000 || last_ldr > millis()) {
        log(String("ldr in:") + String(analogRead(LDR_PIN)) + String(" avg:") + String(ldr_avg) + String(" brightness:") + String(ha_brightness));
        last_ldr = millis();
    }
    #endif // LDR_DEBUG
    #endif // LDR_ENABLED

    static bool effects_disabled = false;
    if (brightness > 35) {
        effects_disabled = false;
    } else if (brightness < 25) {
        effects_disabled = true;
    }

    led_loop(ha_on, ha_hue, ha_saturation, brightness, effects_disabled ? EFFECT_STATIC : ha_effect);

    delay(10);
}
