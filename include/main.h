#ifndef MAIN_H
#define MAIN_H

#include "config.h"

#include <Arduino.h>
#include <time.h>
#include <Bounce2.h>
#include <LittleFS.h>

#ifdef WIFI_ENABLED
    #include <ArduinoOTA.h>

    #ifdef MQTT_ENABLED
        #include <AsyncMqttClient.h>
        #include <ArduinoJson.h>
    #endif

    #ifdef WIFI_AP_ENABLE
        #include <WiFiManager.h>
    #endif
#endif

// Must be included even if WiFi is disabled, it is used to disable WiFi
#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

#include "led.h"
#include "effects.h"
#include "log.h"

#endif
