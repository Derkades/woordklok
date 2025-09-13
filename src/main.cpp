#include "main.h"

#define MQTT_TOPIC_BASE            "woordklok"
#define MQTT_TOPIC_LOG             MQTT_TOPIC_BASE "/" "log"
#define MQTT_TOPIC_HA              MQTT_TOPIC_BASE "/" "ha"
#define MQTT_TOPIC_HA_AVAILABILITY MQTT_TOPIC_HA   "/" "availability"
#define MQTT_TOPIC_HA_STATE        MQTT_TOPIC_HA   "/" "state"
#define MQTT_TOPIC_HA_COMMAND      MQTT_TOPIC_HA   "/" "command"

#define MQTT_QOS_AT_MOST_ONCE 0
#define MQTT_QOS_AT_LEAST_ONCE 1
#define MQTT_QOS_EXACTLY_ONCE 2

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

#ifdef MQTT_ENABLED
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
bool      ha_state_need_publish = true;
#endif

// Home Assistant (MQTT) light settings
bool      ha_on                 = true;
LedEffect ha_effect             = INITIAL_EFFECT;
uint8_t   ha_hue                = INITIAL_HUE;
uint8_t   ha_saturation         = INITIAL_HUE;
uint8_t   ha_brightness         = INITIAL_BRIGHTNESS;

void log(const String &msg) {
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

void connectToWifi() {
    WiFi.setHostname("woordklok");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
    #ifdef MQTT_ENABLED
    connectToMqtt();
    #endif
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
    #ifdef MQTT_ENABLED
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    #endif
    wifiReconnectTimer.once(2, connectToWifi);
}

void setupWifi() {
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    #ifdef MQTT_ENABLED
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    #endif

    connectToWifi();
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
    led_setup();
    configTime(TIMEZONE, "pool.ntp.org");
    startup_animation();
    setupWifi();
}

void loop() {
    ArduinoOTA.handle();

    #ifdef MQTT_ENABLED
    if (millis() % 1024 == 0) {
        mqttClient.publish(MQTT_TOPIC_HA_AVAILABILITY, MQTT_QOS_AT_MOST_ONCE, false, "online");
    }

    if (millis() % 64 == 0 && ha_state_need_publish) {
        publishState();
    }
    #endif

    led_loop(ha_on, ha_hue, ha_saturation, ha_brightness, ha_effect);

    delay(10);
}
