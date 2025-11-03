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

    server.on("/", HTTP_GET, [&]() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/index", HTTP_GET, [&]() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });    
    
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

    // Serve files.html for GET /files
    server.on("/sdcard", HTTP_GET, [&]() {
        handleFileAccess(server);
    });

    // Serve files.html for GET /files
    server.on("/spiffs", HTTP_GET, [&]() {
        handleStorageAccess(server);
    });

    server.on("/getfile", HTTP_GET, [&]() {
        handleFileContent(server);
    });

    // Handle realtime streaming of sensor readings
    server.on("/stream", HTTP_GET, [&]() {
        handleRealtimeStreaming(server);
    });

    // Serve update.html for GET /update
    server.on("/update", HTTP_GET, []() {
        handleGetUpdate(server, String(""));
    });

    // Handle POST /update for OTA firmware update
    server.on("/update", HTTP_POST, []() {
        handlePostUpdate(server);
    }, [&settings]() {
        handlePostUpload(server, settings);
    });

    // Serve testprofile.html for GET /testprofile
    server.on("/testprofile", HTTP_GET, [&]() {
        handleGetTestProfile(server);
    });

    // Handle POST /testprofile
    server.on("/testprofile", HTTP_POST, [&]() {
        handlePostTestProfile(server);
    });

    server.begin();
    Serial.println("Web server started.");
}

void handleWebServer(){
    server.handleClient(); // service any web requests
}

void handleGetSettings(WebServer& server, Settings& settings, bool isSaved) {
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
    html.replace("{{VOLTS_PER_POINT_VOLTAGE}}", String(settings.getVoltsPerPointVoltage(),6));
    html.replace("{{VOLTAGE_OFFSET}}", String(settings.getVoltageOffset()));
    html.replace("{{VOLTS_PER_POINT_CURRENT}}", String(settings.getVoltsPerPointCurrent(),6));
    html.replace("{{CURRENT_OFFSET}}", String(settings.getCurrentOffset()));
    html.replace("{{THRUST_SCALE}}", String(settings.getThrustScale()));
    html.replace("{{THRUST_OFFSET}}", String(settings.getThrustOffset()));
    html.replace("{{MAX_CURRENT}}", String(settings.getMaxCurrent()));
    html.replace("{{MAX_THRUST}}", String(settings.getMaxThrust()));
    html.replace("{{TEST_PHASE_DURATION}}", String(settings.getTestPhaseDuration()));
    html.replace("{{TEST_WARM_DURATION}}", String(settings.getTestWarmDuration())); // Assuming testWarmDuration is defined
    // Check if settings were saved successfully
    if (isSaved) {
        html.replace("{{#DISPLAY_BANNER}}", ""); // Enable the banner
        html.replace("{{/DISPLAY_BANNER}}", "");
    } else {
        html.replace("{{#DISPLAY_BANNER}}", "<!--");
        html.replace("{{/DISPLAY_BANNER}}", "-->");
    }    
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
        settings.setThrustScale(server.arg("thrustScale").toDouble());
        settings.setThrustOffset(server.arg("thrustOffset").toDouble());
        settings.setMaxCurrent(server.arg("maxCurrent").toInt());
        settings.setMaxThrust(server.arg("maxThrust").toInt());
        settings.setTestPhaseDuration(server.arg("testPhaseDuration").toInt());
        settings.setTestWarmDuration(server.arg("testWarmDuration").toInt()); // Assuming testWarmDuration is defined

        settings.saveSettings();
        // server.send(200, "application/json", "{\"status\":\"success\"}");
        handleGetSettings(server, settings, true); // Redirect to GET /settings with success message
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
}

void handleFileAccess(WebServer& server) {

    setCS(SDCARD); // Set the SD card CS pin
    SPI.begin(18,19,23); // Initialize SPI bus
    if (!SD.begin(SD_CS)) {
        server.send(500, "text/plain", "Failed to initialize SD card");
        setCS(TOUCH); // Set the SD card CS pin
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
    setCS(TOUCH); // Set the SD card CS pin
    server.send(200, "application/json", fileList);
}

void handleStorageAccess(WebServer& server) {
    if (!SPIFFS.begin(true)) {
        server.send(500, "text/plain", "Failed to initialize SPIFFS");
        return;
    }
    Serial.println("Handling GET /spiffs request");
    File spiffsFile = SPIFFS.open("/spiffs.html", "r");
    if (!spiffsFile) {
        server.send(500, "text/plain", "Failed to open settings.html");
        return;
    }

    // Read the file content into a String
    String html = spiffsFile.readString();
    spiffsFile.close();


    // Iterate through SPIFFS files and add them with radio buttons
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

    // Replace placeholders with actual settings values
    html.replace("{{FILELIST}}", htmlFileList);

    // Send the generated HTML to the client
    server.send(200, "text/html", html);
}

void handleFileContent(WebServer& server){
    Serial.println("Handling GET /spiffs request");
    if (server.hasArg("filename") && server.hasArg("from")) {

        if(server.arg("from") == "sdcard"){
            if (!SD.begin()) {
                server.send(500, "text/plain", "Failed to initialize SD card");
                return;
            }
            File file = SD.open("/" + server.arg("filename"), "r");
            if (!file) {
                server.send(404, "text/plain", "File not found");
                return;
            }
            else{
                server.streamFile(file, "text/css");
                file.close();
            }
        } else if(server.arg("from") == "spiffs"){
            if (!SPIFFS.begin(true)) {
                server.send(500, "text/plain", "Failed to initialize SPIFFS");
                return;
            }
            else{
                File file = SPIFFS.open("/" + server.arg("filename"), "r");
                if (!file) {
                    server.send(404, "text/plain", "File not found");
                    return;
                }
                else{
                    server.streamFile(file, "text/css");
                    file.close();
                }
            }
        } else {
            server.send(400, "text/plain", "Invalid request");
            return;
        }
    } else {
        server.send(400, "text/plain", "Invalid request");
    }
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

void handleGetUpdate(WebServer& server, const String& message){
    if (motorControl.isRunning()) {
        server.send(500, "text/plain", "Motor is running. Cannot update firmware.");
        return;
    }
    File file = SPIFFS.open("/update.html", "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    // Read the file content into a String
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
    // Send the modified HTML to the client
    server.send(200, "text/html", html); 
}

void handlePostUpdate(WebServer& server){
    // handleGetUpdate(server, (Update.hasError()) ? String("Update Failed") : String("Update Successful. Rebooting..."));
    server.send(200, "text/plain", (Update.hasError()) ? "Update Failed" : "Update Successful. Rebooting...");
    delay(3000);
    ESP.restart();
}

void handlePostUpload(WebServer& server, Settings& settings) {
    HTTPUpload& upload = server.upload();
    static String uploadType; // Store the upload type (firmware or filesystem)
    if (upload.status == UPLOAD_FILE_START) {
        // Get the upload type from the request
        if (server.hasArg("uploadType")) {
            uploadType = server.arg("uploadType");
            Serial.printf("Upload Type: %s\n", uploadType.c_str());
        } else {
            Serial.println("Upload Type not specified. Defaulting to firmware.");
            uploadType = "firmware"; // Default to firmware if not specified
        }

        // Backup settings.json if the upload type is filesystem
        if (uploadType == "filesystem") {
            if (SPIFFS.exists("/settings.json")) {
                settings.loadSettings();
                Serial.println("settings.json backed up successfully.");
            }
        }

        // Start the update process based on the upload type
        if (uploadType == "firmware") {
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) { // Start firmware update
                Update.printError(Serial);
                return;
            }
        } else if (uploadType == "filesystem") {
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) { // Start filesystem update
                Update.printError(Serial);
                return;
            }
        } else {
            Serial.println("Invalid upload type.");
            return;
        }

        Serial.printf("Update: %s\n", upload.filename.c_str());

        TftUpdate tftUpdate(tft);
        tftUpdate.init();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Write the received data to the flash
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { // End and validate the update
            // Restore settings.json if the upload type is filesystem
            if (uploadType == "filesystem") {
                settings.saveSettings();
                Serial.println("settings.json restored successfully.");
            }
            Serial.printf("Update Success: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        Serial.println("Update Aborted");
    }
}

void handleGetTestProfile(WebServer& server, bool isSaved){
    Serial.println("Handling GET /testprofile request");
    File file = SPIFFS.open("/testprofile.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open testprofile.html");
        return;
    }

    // Read the file content into a String
    String html = file.readString();
    file.close();

    // Check if settings were saved successfully
    if (isSaved) {
        html.replace("{{#DISPLAY_BANNER}}", ""); // Enable the banner
        html.replace("{{/DISPLAY_BANNER}}", "");
    } else {
        html.replace("{{#DISPLAY_BANNER}}", "<!--");
        html.replace("{{/DISPLAY_BANNER}}", "-->");
    }    
    // Send the modified HTML to the client
    server.send(200, "text/html", html);
}

void handlePostTestProfile(WebServer& server){
    Serial.println("Handling POST /testprofile request");
    if (server.hasArg("motorName") && server.hasArg("propDiameter") && server.hasArg("propPitch") && server.hasArg("batteryCells")) {
        String motorName = server.arg("motorName");
        int propDiameter = server.arg("propDiameter").toInt();
        int propPitch = server.arg("propPitch").toInt();
        int batteryCells = server.arg("batteryCells").toInt();
        bool isEDF = (server.arg("isEDF") == "true");

        // Validate inputs
        if (motorName.length() < 1 || motorName.length() > 50 || batteryCells < 2 || batteryCells > 8) {
            server.send(400, "text/plain", "Invalid input values");
            return;
        }

        // Save to testprofile.txt on SPIFFS
        File file = SPIFFS.open("/testprofile.json", "w");
        if (!file) {
            server.send(500, "text/plain", "Failed to open testprofile.json for writing");
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
        handleGetTestProfile(server, true); // Redirect to GET /testprofile with success message
    } else {
        server.send(400, "text/plain", "Invalid request");
    }
}