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

    panels[PANEL_VOLTAGE] = Panel("Volts", "Volts", 10, 50, panelWidth, panelHeight, "0.0");
    panels[PANEL_CURRENT] = Panel("Current", "Amps", tft.height() / 3 + 10, 50, panelWidth, panelHeight, "0.0");
    panels[PANEL_THRUST] = Panel("Thrust", "Grams", 2 * (tft.height() / 3) + 10, 50, panelWidth, panelHeight, "0");
    panels[PANEL_POWER] = Panel("Power", "Watts", 10, 50 + panelHeight + 10, panelWidth, panelHeight, "0");
    panels[PANEL_CONSUMPTION] = Panel("Consumption", "mAh", tft.height() / 3 + 10, 50 + panelHeight + 10, panelWidth, panelHeight, "0");
    panels[PANEL_TIME] = Panel("Time", "", 2 * (tft.height() / 3) + 10, 50 + panelHeight + 10, panelWidth, panelHeight, "00:00");
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
    // Draw the throttle background
    tft.drawRect(10, tft.height() - 30, tft.width() - 18, 30, TFT_WHITE);
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
    static int lastThrottlePercent = -1; // Track the last throttle value to avoid redundant updates

    // Only update if the throttle percentage has changed
    if (throttlePercent == lastThrottlePercent) {
        return;
    }

    lastThrottlePercent = throttlePercent; // Update the last throttle value

    // Determine the throttle bar color
    uint16_t barColor = TFT_GREEN;
    if (throttlePercent >= 50 && throttlePercent < 85) {
        barColor = TFT_ORANGE;
    } else if (throttlePercent >= 85) {
        barColor = TFT_RED;
    }

    // Calculate the new bar width
    int barWidth = ((tft.width() - 20) * throttlePercent) / 100; // Adjust for rectangle's left padding

    // Clear the previous bar area if needed
    if (throttlePercent < 100) {
        int clearWidth = ((tft.width() - 20) * (100 - throttlePercent)) / 100;
        tft.fillRect(barWidth + 12, tft.height() - 28, clearWidth - 2, 26, TFT_NAVY);
    }

    // Draw the updated throttle bar
    tft.fillRect(12, tft.height() - 28, barWidth - 2, 26, barColor);

    // Update the throttle percentage text
    tft.setTextColor(TFT_WHITE);
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
    // Update panel values
    updatePanelValue(PANEL_VOLTAGE, String(Voltage,2).c_str());
    updatePanelValue(PANEL_CURRENT, String(Current,1).c_str());   
    updatePanelValue(PANEL_THRUST, String(Thrust).c_str());
    updatePanelValue(PANEL_POWER, String(Power).c_str());
    updatePanelValue(PANEL_CONSUMPTION, String(Consumption).c_str());
    updatePanelValue(PANEL_TIME, Time.c_str());
    drawThrottleIndicator(ThrottlePercent); // Update throttle indicator

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
    static bool errorBoxDisplayed;

    // Check for throttle cut during a running test
    if (currentScreenState == TESTING && motorControl.getThrottleCut()) {
        showStopPressedBox();
        motorControl.stop(); // Optionally stop the motor
        drawThrottleIndicator(0); // Reset throttle indicator if motor is not running
        setButtonState(stopButton, DISABLE);
        setButtonState(startButton, ENABLE);
        currentScreenState = IDLE;
        errorBoxDisplayed = true;
        return;
    }    

    // if (testType == AUTO && !motorControl.isRunning() && currentScreenState == TESTING) {
    //     drawThrottleIndicator(0); // Reset throttle indicator if motor is not running
    //     onStopPressed();
    // }
    if (digitalRead(TOUCH_IRQ) == LOW) {
        unsigned long currentTime = millis();
        if (currentTime - lastTouchTime < debounceDelay) {
            setCS(TOUCH); // Set CS for touch controller
            return; // Ignore touch if within debounce delay
        }
        lastTouchTime = currentTime;

        TS_Point p = ts.getPoint();
        int x = map(p.y, 0, 4095, 0, tft.width()); // Map touch X to screen X
        int y = map(p.x, 0, 4095, 0, tft.height()); // Map touch Y to screen Y
        Serial.println("Touch coordinates: " + String(x) + ", " + String(y));

        setCS(PANEL); // Set CS for TFT panel
        // Check if Exit button is pressed
        if (x >= exitButton.x && x <= exitButton.x + exitButton.width &&
                y >= exitButton.y && y <= exitButton.y + exitButton.height) {
            if (screenChangeCallback) {
                Serial.println("Exit button pressed, changing screen to MAIN_MENU");
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
            // motorControl.setThrottleCut(false); // Reset throttle cut
            onResetPressed();
            if(errorBoxDisplayed){
                errorBoxDisplayed = false;
                init(testType); // Re-initialize the screen
            }
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

    String error;
    if(testType == MANUAL) {
        // Set the motor to manual mode
        motorControl.startManual(error); // Set the motor to manual mode (replace with actual motor control logic)
    } else if(testType == AUTO) {
        // Set the motor to auto mode
        motorControl.startAuto(error); // Set the motor to auto mode (replace with actual motor control logic)
    }   
    currentScreenState = TESTING; // Update the screen state to TESTING

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

    motorControl.stop(); // Stop the motor (replace with actual motor control logic)
    currentScreenState = IDLE; // Update the screen state to IDLE
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

    motorControl.reset(); // Reset the motor (replace with actual motor control logic)
    currentScreenState = IDLE; // Update the screen state to IDLE

    // Update panel values
    updatePanelValue(PANEL_CONSUMPTION, "0"); // Reset consumption
    updatePanelValue(PANEL_TIME, "00:00");  // Reset time  
    Serial.println("Stop button pressed. Timer stopped.");
}

void TftMotorTest::showStopPressedBox() {
    setCS(PANEL);
    int boxWidth = tft.width() / 2;
    int boxHeight = 80;
    int boxX = (tft.width() - boxWidth) / 2;
    int boxY = (tft.height() - boxHeight) / 2;

    // Draw red box with border
    tft.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 12, TFT_RED);
    tft.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 12, TFT_WHITE);

    // Draw message centered in box
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("STOP button pressed", boxX + boxWidth / 2, boxY + boxHeight / 2);

    setCS(TOUCH);
}