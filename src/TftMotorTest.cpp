#include "TftMotorTest.h"

// Timer interrupt handler
void IRAM_ATTR TftMotorTest::onTimer(void* arg) {
    setCS(PANEL); // Set CS for TFT panel
    TftMotorTest* instance = static_cast<TftMotorTest*>(arg);
    instance->updatePanelValues(); // Call the updatePanels method
    setCS(TOUCH); // Set CS for touch controller
}

TftMotorTest::TftMotorTest(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback),
      startButton{"Start", 10, 5, 80, 30, ENABLE},
      stopButton{"Stop", 100, 5, 80, 30, DISABLE},
      resetButton{"Reset", 190, 5, 80, 30, ENABLE},
      exitButton{"Exit", tft.height() - 90, 5, 80, 30, ENABLE},
      panels{Panel("", "", 0, 0, 0, 0, ""), Panel("", "", 0, 0, 0, 0, ""), 
             Panel("", "", 0, 0, 0, 0, ""), Panel("", "", 0, 0, 0, 0, ""), 
             Panel("", "", 0, 0, 0, 0, ""), Panel("", "", 0, 0, 0, 0, "")} {
    // Initialize panel details with width and height
    int panelWidth = tft.height() / 3 - 15;
    int panelHeight = (tft.width() - 70) / 2 - 15;

    panels[0] = Panel("Volts", "Volts", 10, 50, panelWidth, panelHeight, "0.0");
    panels[1] = Panel("Current", "Amps", tft.height() / 3 + 10, 50, panelWidth, panelHeight, "0.0");
    panels[2] = Panel("Thrust", "Grams", 2 * (tft.height() / 3) + 10, 50, panelWidth, panelHeight, "0");
    panels[3] = Panel("Power", "Watts", 10, 50 + panelHeight + 10, panelWidth, panelHeight, "0");
    panels[4] = Panel("Consumption", "mAh", tft.height() / 3 + 10, 50 + panelHeight + 10, panelWidth, panelHeight, "0");
    panels[5] = Panel("Time", "", 2 * (tft.height() / 3) + 10, 50 + panelHeight + 10, panelWidth, panelHeight, "00:00");
}

void TftMotorTest::init(TestType testType) {
    // Chip select PANEL
    setCS(PANEL);
    this->testType = testType; // Set the test type

    // Set background color
    tft.fillScreen(TFT_NAVY);

    // Draw the ribbon
    drawRibbon();

    // Draw the throttle indicator
    drawThrottleIndicator(0); // Start with 0% throttle

    // Draw the panels
    drawPanels();

    // Chip select TOUCH
    setCS(TOUCH);
}

void TftMotorTest::drawRibbon() {
    tft.fillRect(0, 0, tft.width(), 40, TFT_DARKCYAN);

    // Draw buttons
    drawButton(startButton);
    drawButton(stopButton);
    drawButton(resetButton);
    drawButton(exitButton);
}

void TftMotorTest::drawButton(const Button& button) {
    uint16_t bgColor = (button.state == ENABLE) ? TFT_GREEN : TFT_LIGHTGREY;
    uint16_t textColor = (button.state == ENABLE) ? TFT_BLACK : TFT_WHITE;

    tft.fillRoundRect(button.x, button.y, button.width, button.height, 5, bgColor);
    tft.setTextColor(textColor);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(button.label, button.x + button.width / 2, button.y + button.height / 2 - 7);
}

void TftMotorTest::setButtonState(Button& button, ButtonState state) {
    button.state = state;
    drawButton(button);
}

void TftMotorTest::drawThrottleIndicator(int throttlePercent) {
    // Draw the throttle background
    tft.drawRect(10, tft.height() - 30, tft.width() - 10, 30, TFT_WHITE);

    // Determine the throttle bar color
    uint16_t barColor = TFT_GREEN;
    if (throttlePercent >= 50 && throttlePercent < 85) {
        barColor = TFT_ORANGE;
    } else if (throttlePercent >= 85) {
        barColor = TFT_RED;
    }

    // Draw the throttle bar
    int barWidth = (tft.width() * throttlePercent) / 100;
    tft.fillRect(0, tft.height() - 30, barWidth, 30, barColor);

    // Display the throttle percentage
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    String throttleValue = String(throttlePercent) + '%';
    tft.drawString(throttleValue.c_str(), tft.width() / 2, tft.height() - 22);
}

void TftMotorTest::drawPanels() {
    for (int i = 0; i < NUM_PANELS; i++) {
        drawPanel(panels[i]);
    }
}

void TftMotorTest::drawPanel(const Panel& panel) {
    // Draw panel background
    tft.fillRoundRect(panel.x, panel.y, panel.width, panel.height, 5, TFT_YELLOW);

    // Draw panel border
    tft.drawRoundRect(panel.x, panel.y, panel.width, panel.height, 5, TFT_BLACK);

    // Display label at the top-left
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(panel.label, panel.x + 5, panel.y + 5);

    // Display value in the middle
    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(panel.value, panel.x + panel.width / 2, panel.y + panel.height / 2 - 10);

    // Display unit at the bottom
    tft.setTextSize(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(panel.unit, panel.x + panel.width / 2, panel.y + panel.height - 5);
}

// Method to update only the values within the panels
void TftMotorTest::updatePanelValues() {
    // Simulate reading sensor values (replace with actual sensor readings)
    String volts = String(random(110, 130) / 10.0, 1); // Simulated voltage
    String current = String(random(10, 50) / 10.0, 1); // Simulated current
    String thrust = String(random(400, 600));          // Simulated thrust
    String power = String(volts.toInt() * current.toInt());             // Simulated power
    String consumption = String(random(100, 200));     // Simulated consumption
    String time = "00:" + String(random(10, 59));      // Simulated time

    // Update panel values
    updatePanelValue(0, volts.c_str());
    updatePanelValue(1, current.c_str());
    updatePanelValue(2, thrust.c_str());
    updatePanelValue(3, power.c_str());
    updatePanelValue(4, consumption.c_str());
    updatePanelValue(5, time.c_str());

    Serial.println("Panel values updated.");
}

void TftMotorTest::updatePanelValue(int panelIndex, const char* value) {
    if (panelIndex < 0 || panelIndex >= NUM_PANELS) return; // Ensure valid index

    Panel& panel = panels[panelIndex];
    strncpy(panel.value, value, sizeof(panel.value) - 1); // Update the value
    panel.value[sizeof(panel.value) - 1] = '\0';          // Ensure null termination

    // Clear the previous value
    tft.fillRect(panel.x + 5, panel.y + panel.height / 2 - 10, panel.width - 10, 20, TFT_YELLOW);

    // Display the new value in the middle of the panel
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(panel.value, panel.x + panel.width / 2, panel.y + panel.height / 2 - 10);
}

void TftMotorTest::handleTouch() {
    if (digitalRead(TOUCH_IRQ) == LOW) {
        unsigned long currentTime = millis();
        if (currentTime - lastTouchTime < debounceDelay) {
            setCS(TOUCH); // Set CS for touch controller
            return; // Ignore touch if within debounce delay
        }
        lastTouchTime = currentTime;

        TS_Point p = ts.getPoint();
        int x = map(p.y, 4095, 0, 0, tft.width()); // Map touch X to screen X
        int y = map(p.x, 4095, 0, 0, tft.height()); // Map touch Y to screen Y
        Serial.println("Touch coordinates: " + String(x) + ", " + String(y));

        setCS(PANEL); // Set CS for TFT panel
        // Check if Exit button is pressed
        if (x >= exitButton.x && x <= exitButton.x + exitButton.width &&
                y >= exitButton.y && y <= exitButton.y + exitButton.height) {
            if (screenChangeCallback) {
                Serial.println("Exit button pressed, changing screen to MAIN_MENU");
                // Stop and delete the timer
                if (updateTimer) {
                    esp_timer_stop(updateTimer);
                    esp_timer_delete(updateTimer);
                    updateTimer = nullptr;
                }
                screenChangeCallback(MAIN_MENU);
            }
        }

        // Handle Start button
        if (x >= startButton.x && x <= startButton.x + startButton.width &&
            y >= startButton.y && y <= startButton.y + startButton.height &&
                startButton.state == ENABLE) {
            Serial.println("Start button pressed");
            onStartPressed();
        }

        // Handle Stop button
        if (x >= stopButton.x && x <= stopButton.x + stopButton.width &&
            y >= stopButton.y && y <= stopButton.y + stopButton.height &&
                stopButton.state == ENABLE) {
            Serial.println("Stop button pressed");
            onStopPressed();
        }

        // Handle Reset button
        if (x >= resetButton.x && x <= resetButton.x + resetButton.width &&
            y >= resetButton.y && y <= resetButton.y + resetButton.height &&
                resetButton.state == ENABLE) {
            Serial.println("Reset button pressed");
            onResetPressed();
        }
        setCS(TOUCH); // Set CS for touch controller
    }
}

void TftMotorTest::onStartPressed() {
    setButtonState(startButton, DISABLE);
    setButtonState(stopButton, ENABLE);
    // Add custom logic for Start button
    // Create and start the timer
    const esp_timer_create_args_t timerArgs = {
        .callback = &TftMotorTest::onTimer,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "updateTimer"
    };
    esp_timer_create(&timerArgs, &updateTimer);
    esp_timer_start_periodic(updateTimer, 500000); // 1 second (1,000,000 microseconds)

    Serial.println("Start button pressed. Timer started.");
}

void TftMotorTest::onStopPressed() {
    setButtonState(stopButton, DISABLE);
    setButtonState(startButton, ENABLE);
    // Add custom logic for Stop button
    // Stop and delete the timer
    if (updateTimer) {
        esp_timer_stop(updateTimer);
        esp_timer_delete(updateTimer);
        updateTimer = nullptr;
    }

    Serial.println("Stop button pressed. Timer stopped.");
}

void TftMotorTest::onResetPressed() {
    setButtonState(startButton, ENABLE);
    setButtonState(stopButton, DISABLE);
    // Add custom logic for Reset button
    // Stop and delete the timer
    if (updateTimer) {
        esp_timer_stop(updateTimer);
        esp_timer_delete(updateTimer);
        updateTimer = nullptr;
    }

    Serial.println("Stop button pressed. Timer stopped.");
}