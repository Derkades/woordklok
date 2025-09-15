#ifndef MAIN_H
#define MAIN_H

#include "config.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <time.h>

#ifdef MQTT_ENABLED
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#endif

#ifdef WIFI_AP
#include <WiFiManager.h>
#endif

#include "led.h"
#include "effects.h"
#include "log.h"

#endif
