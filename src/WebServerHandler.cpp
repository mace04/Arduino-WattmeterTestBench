#include "WebServerHandler.h"

AsyncWebServer server(80);
AsyncEventSource events("/events");  // SSE endpoint

extern TFT_eSPI tft;
extern MotorControl motorControl;

void initWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nWiFi connection failed. Starting hotspot...");
        WiFi.softAP("ESP32_Hotspot", "12345678");
        Serial.println("Hotspot started. IP address: " + WiFi.softAPIP().toString());
    } else {
        Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
    }
}

void initWebServer(Settings& settings) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    server.addHandler(&events);  // Add SSE handler

    // Serve root
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // Serve settings.html for GET /settings
    server.on("/settings", HTTP_GET, [&settings](AsyncWebServerRequest *request) {
        handleGetSettings(request, settings, false);
    });

    // Serve style.css for GET /style.css
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });

    // Serve index.js for GET /index.js
    server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.js", "application/javascript");
    });

    // Handle POST /settings to update settings
    server.on("/settings", HTTP_POST, [&settings](AsyncWebServerRequest *request) {
        handlePostSettings(request, settings);
    });

    // Serve spiffs.html for GET /spiffs
    server.on("/spiffs", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleStorageAccess(request);
    });

    // Get file content
    server.on("/getfile", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleFileContent(request);
    });

    // Handle realtime streaming of sensor readings
    server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleRealtimeStreaming(request);
    });

    // Serve update.html for GET /update
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleGetUpdate(request, "");
    });

    // Handle POST /update for OTA firmware update
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        handlePostUpdate(request);
    }, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        handlePostUpload(request, filename, index, data, len, final);
    });

    // Serve testprofile.html for GET /testprofile
    server.on("/testprofile", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleGetTestProfile(request, false);
    });

    // Handle POST /testprofile
    server.on("/testprofile", HTTP_POST, [](AsyncWebServerRequest *request) {
        handlePostTestProfile(request);
    });

    server.begin();
    Serial.println("Async Web server started.");
}

void handleWebServer() {
    // AsyncWebServer handles requests automatically; no explicit call needed
}

void sendDebugEvent(const String& msg) {
    events.send(msg.c_str(), "debug", millis());
}

void sendLogEvent(const String& msg) {
    events.send(msg.c_str(), "log", millis());
}

void handleGetSettings(AsyncWebServerRequest *request, Settings& settings, bool isSaved) {
    Serial.println("Handling GET /settings request");
    File file = SPIFFS.open("/settings.html", "r");
    if (!file) {
        request->send(404, "text/plain", "Settings file not found");
        return;
    }

    String html = file.readString();
    file.close();

    // Replace placeholders with actual settings values
    html.replace("{{SSID}}", settings.getSSID());
    html.replace("{{PASSWORD}}", settings.getPassword());
    html.replace("{{VOLTS_PER_POINT_VOLTAGE}}", String(settings.getVoltsPerPointVoltage(), 6));
    html.replace("{{VOLTAGE_OFFSET}}", String(settings.getVoltageOffset()));
    html.replace("{{VOLTS_PER_POINT_CURRENT}}", String(settings.getVoltsPerPointCurrent(), 6));
    html.replace("{{CURRENT_OFFSET}}", String(settings.getCurrentOffset()));
    html.replace("{{THRUST_SCALE}}", String(settings.getThrustScale()));
    html.replace("{{THRUST_OFFSET}}", String(settings.getThrustOffset()));
    html.replace("{{MAX_CURRENT}}", String(settings.getMaxCurrent()));
    html.replace("{{MAX_THRUST}}", String(settings.getMaxThrust()));
    html.replace("{{TEST_PHASE_DURATION}}", String(settings.getTestPhaseDuration()));
    html.replace("{{TEST_WARM_DURATION}}", String(settings.getTestWarmDuration()));

    if (isSaved) {
        html.replace("{{#DISPLAY_BANNER}}", ""); // Enable the banner
        html.replace("{{/DISPLAY_BANNER}}", "");
    } else {
        html.replace("{{#DISPLAY_BANNER}}", "<!--");
        html.replace("{{/DISPLAY_BANNER}}", "-->");
    }

    request->send(200, "text/html", html);
}

void handlePostSettings(AsyncWebServerRequest *request, Settings& settings) {
    Serial.println("Handling POST /settings request");
    if (request->hasArg("ssid") && request->hasArg("password")) {
        // Handle form-encoded data
        settings.setSSID(request->arg("ssid"));
        settings.setPassword(request->arg("password"));
        settings.setVoltsPerPointVoltage(request->arg("voltsPerPointVoltage").toDouble());
        settings.setVoltageOffset(request->arg("voltageOffset").toDouble());
        settings.setVoltsPerPointCurrent(request->arg("voltsPerPointCurrent").toDouble());
        settings.setCurrentOffset(request->arg("currentOffset").toDouble());
        settings.setThrustScale(request->arg("thrustScale").toDouble());
        settings.setThrustOffset(request->arg("thrustOffset").toDouble());
        settings.setMaxCurrent(request->arg("maxCurrent").toInt());
        settings.setMaxThrust(request->arg("maxThrust").toInt());
        settings.setTestPhaseDuration(request->arg("testPhaseDuration").toInt());
        settings.setTestWarmDuration(request->arg("testWarmDuration").toInt()); // Assuming testWarmDuration is defined

        settings.saveSettings();
        // server.send(200, "application/json", "{\"status\":\"success\"}");
        handleGetSettings(request, settings, true); // Redirect to GET /settings with success message
    } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
}

void handleStorageAccess(AsyncWebServerRequest *request) {
    if (!SPIFFS.begin(true)) {
        request->send(500, "text/plain", "SPIFFS Mount Failed");
        return;
    }

    Serial.println("Handling GET /spiffs request");
    File spiffsFile = SPIFFS.open("/spiffs.html", "r");
    if (!spiffsFile) {
        request->send(404, "text/plain", "SPIFFS file not found");
        return;
    }

    String html = spiffsFile.readString();
    spiffsFile.close();

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    String htmlFileList = "";
    while (file) {
        htmlFileList += "<div class=\"file-item\">";
        htmlFileList += "<input type=\"radio\" name=\"fileRadio\" value=\"" + String(file.name()) + "\">";
        htmlFileList += "<label>" + String(file.name()) + "</label>";
            htmlFileList += "</div>";
        file = root.openNextFile();
    }

    html.replace("{{FILELIST}}", htmlFileList);
    request->send(200, "text/html", html);
}

void handleFileContent(AsyncWebServerRequest *request) {
    Serial.println("Handling GET /getfile request");

    if (request->hasArg("filename") && request->hasArg("from")) {
        String filename = request->arg("filename");
        String from = request->arg("from");

        if (from == "spiffs") {
            File file = SPIFFS.open("/" + filename, "r");
            if (!file) {
                request->send(404, "text/plain", "File not found");
                return;
            }
            String content = file.readString();
            file.close();
            request->send(200, "text/plain", content);
        }
    } else {
        request->send(400, "text/plain", "Missing required parameters");
    }
}

void handleRealtimeStreaming(AsyncWebServerRequest *request) {
    String csvData = "Voltage,Current,Weight,Watts\n";
    csvData += String(readVoltageSensor()) + ",";
    csvData += String(readCurrentSensor()) + ",";
    csvData += String(readWeightSensor()) + ",";
    csvData += String(readVoltageSensor() * readCurrentSensor()) + "\n";

    request->send(200, "text/csv", csvData);
    Serial.println("Realtime streaming triggered.");
}

void handleGetUpdate(AsyncWebServerRequest *request, const String& message, int code) {
    if (motorControl.isRunning() && code != 200) {
        // request->send(403, "text/plain", "Motor is running. Cannot update firmware.");
        handleGetUpdate(request, "Motor is running. Cannot update firmware.", 403);
        return;
    }

    File file = SPIFFS.open("/update.html", "r");
    if (!file) {
        request->send(404, "text/plain", "File not found");
        return;
    }

    String html = file.readString();
    file.close();

    // Check if settings were saved successfully
    if (!message.isEmpty()) {
        html.replace("{{#DISPLAY_BANNER}}", ""); // Enable the banner
        html.replace("{{/DISPLAY_BANNER}}", "");
        html.replace("{{BANNER_MESSAGE}}", message); // Replace with the message
    } else {
        html.replace("{{#DISPLAY_BANNER}}", "<!--");
        html.replace("{{/DISPLAY_BANNER}}", "-->");
    }

    request->send(code, "text/html", html);
}

void handlePostUpdate(AsyncWebServerRequest *request) {
    tft.fillScreen(TFT_NAVY);
    String response = (Update.hasError()) ? "Update Failed" : "Update Successful. Rebooting...";
    request->send(200, "text/plain", response);
    delay(3000);
    ESP.restart();
}

void handlePostUpload(AsyncWebServerRequest *request, const String& filename, size_t index, 
                      uint8_t *data, size_t len, bool final) {
    static uint32_t updateSize = 0;
    static String uploadType = "";

    if (index == 0) {
        Serial.printf("Upload Start: %s\n", filename.c_str());
        updateSize = 0;

        TftUpdate tftUpdate(tft);
        tftUpdate.init();

        // Determine upload type from HTTP argument
        if (request->hasArg("uploadType")) {
            uploadType = request->arg("uploadType");
        } else {
            uploadType = "firmware"; // default to firmware
        }

        Serial.printf("Upload Type: %s\n", uploadType.c_str());

        // Handle firmware update
        if (uploadType == "firmware") {
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                Update.printError(Serial);
                request->send(500, "text/plain", "OTA begin failed");
                return;
            }
            Serial.println("Started firmware update...");
        }
        // Handle filesystem upload
        else if (uploadType == "filesystem") {
            // Create or open file in SPIFFS
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
                Update.printError(Serial);
                request->send(500, "text/plain", "OTA begin failed");
                return;
            }
            Serial.println("Started filesystem update...");        }
        else {
            request->send(400, "text/plain", "Invalid uploadType parameter. Use 'firmware' or 'filesystem'");
            return;
        }
    }

    if (!Update.hasError()) {
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
        }
        updateSize += len;
    }

    if (final) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes\nRebooting...\n", updateSize);
            request->send(200, "text/html", 
                    "<html><body><p>Firmware update successful!</p>"
                    "<p>Rebooting ESP32...</p>"
                    "<meta http-equiv='refresh' content='3; url=/' />"
                    "</body></html>");
            delay(3000);
            ESP.restart();
        } else {
            Update.printError(Serial);
            request->send(500, "text/plain", "OTA end failed");
        }
    }
}

void handleGetTestProfile(AsyncWebServerRequest *request, bool isSaved) {
    File file = SPIFFS.open("/testprofile.html", "r");
    if (!file) {
        request->send(404, "text/plain", "Test profile file not found");
        return;
    }

    String html = file.readString();
    file.close();

    if (isSaved) {
        html.replace("{{#DISPLAY_BANNER}}", ""); // Enable the banner
        html.replace("{{/DISPLAY_BANNER}}", "");
    } else {
        html.replace("{{#DISPLAY_BANNER}}", "<!--");
        html.replace("{{/DISPLAY_BANNER}}", "-->");
    }

    request->send(200, "text/html", html);
}

void handlePostTestProfile(AsyncWebServerRequest *request) {
    // Handle test profile updates here
    request->send(200, "text/html", "<meta http-equiv='refresh' content='2; url=/testprofile' /><p>Test profile saved!</p>");


    Serial.println("Handling POST /testprofile request");
    if (request->hasArg("motorName") && request->hasArg("propDiameter") && request->hasArg("propPitch") && request->hasArg("batteryCells")) {
        String motorName = request->arg("motorName");
        int propDiameter = request->arg("propDiameter").toInt();
        int propPitch = request->arg("propPitch").toInt();
        int batteryCells = request->arg("batteryCells").toInt();
        bool isEDF = (request->arg("isEDF") == "true");

        // Validate inputs
        if (motorName.length() < 1 || motorName.length() > 50 || batteryCells < 2 || batteryCells > 8) {
            request->send(400, "text/plain", "Invalid input values");
            return;
        }

        // Save to testprofile.txt on SPIFFS
        File file = SPIFFS.open("/testprofile.json", "w");
        if (!file) {
            request->send(500, "text/plain", "Failed to open testprofile.json for writing");
            return;
        }
        file.printf("{\"motor\": \"%s\", \"diameter\": %d, \"pitch\": %d, \"cells\": %d}", motorName.c_str(), propDiameter, propPitch, batteryCells);
        file.close();

        testProfile.name = motorName;
        testProfile.isEDF = isEDF;
        testProfile.diameter = propDiameter;
        testProfile.pitch = propPitch;
        testProfile.cells = batteryCells;

        Serial.println("Test profile saved successfully.");
        handleGetTestProfile(request, true); // Redirect to GET /testprofile with success message
    } else {
        request->send(400, "text/plain", "Invalid request");
    }
}