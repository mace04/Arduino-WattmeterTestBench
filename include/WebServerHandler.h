#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <SD.h>
#include "Settings.h"
#include "sensors.h"
#include "TftUpdate.h"
#include "motorControl.h"
#include "tft_config.h"

extern Profile testProfile;

// Function declarations
void initWiFi(const char* ssid, const char* password);
void initWebServer(Settings& settings);
void handleWebServer();
void sendDebugEvent(const String& msg);
void sendErrorEvent(const String& msg);
void sendLogEvent(const String& msg);
void handleGetUpdate(AsyncWebServerRequest *request, const String& message, int code = 200);
void handlePostUpdate(AsyncWebServerRequest *request);
void handlePostUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleGetSettings(AsyncWebServerRequest *request, Settings& settings, bool isSaved = false);
void handlePostSettings(AsyncWebServerRequest *request, Settings& settings);
void handleFileAccess(AsyncWebServerRequest *request);
void handleStorageAccess(AsyncWebServerRequest *request);
void handleFileContent(AsyncWebServerRequest *request);
void handleRealtimeStreaming(AsyncWebServerRequest *request);
void handleGetTestProfile(AsyncWebServerRequest *request, bool isSaved = false);
void handlePostTestProfile(AsyncWebServerRequest *request);
#endif // WEBSERVERHANDLER_H