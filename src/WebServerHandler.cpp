#include "WebServerHandler.h"

WebServer server(80);
extern TFT_eSPI tft;
extern MotorControl motorControl;


void initWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        delay(1000);
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
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Serve settings.html for GET /settings
    server.on("/settings", HTTP_GET, [&]() {
        handleGetSettings(server, settings);
    });

    // Serve style.css for GET /style.css
    server.on("/style.css", HTTP_GET, []() {
        File file = SPIFFS.open("/style.css", "r");
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/css");
        file.close();
    });

    // Serve index.js for GET /index.js
    server.on("/index.js", HTTP_GET, []() {
        File file = SPIFFS.open("/index.js", "r");
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "application/javascript");
        file.close();
    });

    // Handle POST /settings to update settings
    server.on("/settings", HTTP_POST, [&]() {
        handlePostSettings(server, settings);
    });

    // Handle OTA firmware update
    server.on("/ota", HTTP_POST, []() {
        handleOTAUpdate();
    });

    // Serve files.html for GET /files
    server.on("/files", HTTP_GET, [&]() {
        handleFileAccess(server);
    });

    // Handle realtime streaming of sensor readings
    server.on("/stream", HTTP_GET, [&]() {
        handleRealtimeStreaming(server);
    });

    // Serve update.html for GET /update
    server.on("/update", HTTP_GET, []() {
        if (motorControl.isRunning()) {
            server.send(500, "text/plain", "Motor is running. Cannot update firmware.");
            return;
        }
        File file = SPIFFS.open("/update.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    // Handle POST /update for OTA firmware update
    server.on("/update", HTTP_POST, []() {
        server.send(200, "text/plain", (Update.hasError()) ? "Update Failed" : "Update Successful. Rebooting...");
        delay(1000);
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with unknown size
                Update.printError(Serial);
                return;
            }
            Serial.printf("Update: %s\n", upload.filename.c_str());
            TftUpdate tftUpdate(tft);
            tftUpdate.init();
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with unknown size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            // Write the received data to the flash
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { // End and validate the update
                Serial.printf("Update Success: %u bytes\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
            Update.abort();
            Serial.println("Update Aborted");
        }
    });

    server.begin();
    Serial.println("Web server started.");
}

void handleWebServer(){
    server.handleClient(); // service any web requests
}

void handleGetSettings(WebServer& server, Settings& settings) {
    Serial.println("Handling GET /settings request");
    File file = SPIFFS.open("/settings.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open settings.html");
        return;
    }

    // Read the file content into a String
    String html = file.readString();
    file.close();

    // Replace placeholders with actual settings values
    html.replace("{{SSID}}", settings.getSSID());
    html.replace("{{PASSWORD}}", settings.getPassword());
    html.replace("{{VOLTS_PER_POINT_VOLTAGE}}", String(settings.getVoltsPerPointVoltage()));
    html.replace("{{VOLTAGE_OFFSET}}", String(settings.getVoltageOffset()));
    html.replace("{{VOLTS_PER_POINT_CURRENT}}", String(settings.getVoltsPerPointCurrent()));
    html.replace("{{CURRENT_OFFSET}}", String(settings.getCurrentOffset()));
    html.replace("{{THRUST_OFFSET}}", String(settings.getThrustOffset()));
    html.replace("{{MAX_CURRENT}}", String(settings.getMaxCurrent()));
    html.replace("{{MAX_THRUST}}", String(settings.getMaxThrust()));
    html.replace("{{TEST_PHASE_DURATION}}", String(settings.getTestPhaseDuration()));

    // Send the modified HTML to the client
    server.send(200, "text/html", html);
}

void handlePostSettings(WebServer& server, Settings& settings) {
    Serial.println("Handling POST /settings request");
    if (server.hasArg("ssid") && server.hasArg("password")) {
        // Handle form-encoded data
        settings.setSSID(server.arg("ssid"));
        settings.setPassword(server.arg("password"));
        settings.setVoltsPerPointVoltage(server.arg("voltsPerPointVoltage").toDouble());
        settings.setVoltageOffset(server.arg("voltageOffset").toDouble());
        settings.setVoltsPerPointCurrent(server.arg("voltsPerPointCurrent").toDouble());
        settings.setCurrentOffset(server.arg("currentOffset").toDouble());
        settings.setThrustOffset(server.arg("thrustOffset").toDouble());
        settings.setMaxCurrent(server.arg("maxCurrent").toInt());
        settings.setMaxThrust(server.arg("maxThrust").toInt());
        settings.setTestPhaseDuration(server.arg("testPhaseDuration").toInt());

        settings.saveSettings();
        server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
}

void handleOTAUpdate() {
    // Placeholder for OTA firmware update logic
    Serial.println("OTA update triggered.");
}

void handleFileAccess(WebServer& server) {
    if (!SD.begin()) {
        server.send(500, "text/plain", "Failed to initialize SD card");
        return;
    }

    String fileList = "[";
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
        if (fileList.length() > 1) fileList += ",";
        fileList += "\"" + String(file.name()) + "\"";
        file = root.openNextFile();
    }
    fileList += "]";

    server.send(200, "application/json", fileList);
}

void handleRealtimeStreaming(WebServer& server) {
    String csvData = "Voltage,Current,Weight,Watts\n";
    csvData += String(readVoltageSensor()) + ",";
    csvData += String(readCurrentSensor()) + ",";
    csvData += String(readWeightSensor()) + ",";
    csvData += String(readVoltageSensor() * readCurrentSensor()) + "\n"; // Example calculation for watts
    server.send(200, "text/csv", csvData);
    Serial.println("Realtime streaming triggered.");
}