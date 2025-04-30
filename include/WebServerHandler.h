#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <SD.h>
#include "Settings.h"
#include "sensors.h"
#include "TftUpdate.h"
#include "motorControl.h"

// Function declarations
void initWiFi(const char* ssid, const char* password);
void initWebServer(Settings& settings);
void handleWebServer();
void handleGetUpdate(WebServer& server, const String& message);
void handlePostUpdate(WebServer& server);
void handlePostUpload(WebServer& server, Settings& settings);
void handleGetSettings(WebServer& server, Settings& settings, bool isSaved = false);
void handlePostSettings(WebServer& server, Settings& settings);
void handleOTAUpdate();
void handleFileAccess(WebServer& server);
void handleRealtimeStreaming(WebServer& server);
#endif // WEBSERVERHANDLER_H