#include "TftCalibrate.h"

TftCalibrate::TftCalibrate(TFT_eSPI& tft, XPT2046_Touchscreen& ts)
    : tft(tft), ts(ts),
      panelVoltage("Volts", "V", 10, 10, 150, 70, "0.0"),
      panelCurrent("Current", "A", 10, 90, 150, 70, "0.0"),
      panelThrust("Thrust", "gr", 10, 170, 150, 70, "0"),
      voltsPerPointVoltageBox{"Scale", 170, 45, 90, 30, ""},
      voltageOffsetBox{"Offset", 270, 45, 90, 30, ""},
      voltsPerPointCurrentBox{"Scale", 170, 125, 90, 30, ""},
      currentOffsetBox{"Offset",270, 125, 90, 30, ""},
      thrustOffsetBox{"Offset",170, 205, 90, 30, ""},
      resetButton{"Restart", tft.height() / 2 - 40, tft.width() - 50, 110, 40}
{
}

void TftCalibrate::init(bool calibrateEsc) {
    pinMode(THROTTLE_CONTROL_PIN, INPUT);

     ESP32PWM::allocateTimer(0);
     ESP32PWM::allocateTimer(1);
     ESP32PWM::allocateTimer(2);
     ESP32PWM::allocateTimer(3);
     servo.setPeriodHertz(50);    // standard 50 hz servo
     servo.attach(ESC_OUTPUT_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
     Serial.println("throttle control pin reading: " + String(analogRead(THROTTLE_CONTROL_PIN)));
     if (calibrateEsc)
     {
        servo.writeMicroseconds(2000); // Set initial ESC signal to maximum throttle
        delay(2000); // Wait for 2 seconds to ensure ESC registers max throttle
        servo.writeMicroseconds(1000); // Set initial ESC signal to minimum throttle
        delay(2000); // Wait for 2 seconds to ensure ESC registers max throttle
     }


    tft.fillScreen(TFT_NAVY);

    drawPanel(panelVoltage);
    drawPanel(panelCurrent);
    drawPanel(panelThrust);

    voltsPerPointVoltageBox.value = String(settings.getVoltsPerPointVoltage(), 4);
    voltageOffsetBox.value = String(settings.getVoltageOffset(), 2);
    voltsPerPointCurrentBox.value = String(settings.getVoltsPerPointCurrent(), 4);
    currentOffsetBox.value = String(settings.getCurrentOffset(), 2);
    thrustOffsetBox.value = String(settings.getThrustOffset(), 1);

    drawTextBox(voltsPerPointVoltageBox);
    drawTextBox(voltageOffsetBox);
    drawTextBox(voltsPerPointCurrentBox);
    drawTextBox(currentOffsetBox);
    drawTextBox(thrustOffsetBox);

    drawButton(resetButton);

    String error;
    motorControl.startManual(error);
    if (!error.isEmpty()) {
        Serial.println("Motor start error: " + error);
    }
}

void TftCalibrate::drawPanel(const Panel& panel) {
    tft.fillRoundRect(panel.x, panel.y, panel.width, panel.height, 5, TFT_YELLOW);
    tft.drawRoundRect(panel.x, panel.y, panel.width, panel.height, 5, TFT_BLACK);

    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(panel.label, panel.x + 5, panel.y + 5);

    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(panel.value, panel.x + panel.width / 2, panel.y + panel.height / 2 - 10);

    tft.setTextSize(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(panel.unit, panel.x + panel.width / 2, panel.y + panel.height - 5);
}

void TftCalibrate::updatePanel(const Panel& panel) {
    tft.fillRoundRect(panel.x + 5, panel.y + 20, panel.width - 10, panel.height - 40, 5, TFT_YELLOW);

    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(panel.value, panel.x + panel.width / 2, panel.y + panel.height / 2 - 10);
}
void TftCalibrate::drawTextBox(const TextBox& box) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(box.label, box.x, box.y - 20);
    tft.fillRect(box.x, box.y, box.width, box.height, TFT_LIGHTGREY);
    tft.drawRect(box.x, box.y, box.width, box.height, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(box.value, box.x + box.width / 2, box.y + box.height / 2);
}

void TftCalibrate::drawButton(const Button& button) {
    tft.fillRoundRect(button.x, button.y, button.width, button.height, 5, TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(button.label, button.x + button.width / 2, button.y + button.height / 2 - 7);
}

void TftCalibrate::handle() {
    static unsigned long lastPanelUpdate = 0;
    servo.writeMicroseconds(map(analogRead(THROTTLE_CONTROL_PIN), 0, 4095, 1000, 2000));

    // Update panel values once every second
    unsigned long now = millis();
    if (now - lastPanelUpdate >= 500) {
        // updatePanelValues();
        lastPanelUpdate = now;

        panelVoltage.value[0] = '\0';
        panelCurrent.value[0] = '\0';
        panelThrust.value[0] = '\0';

        snprintf(panelVoltage.value, sizeof(panelVoltage.value), "%.2f", readVoltageSensor());
        snprintf(panelCurrent.value, sizeof(panelCurrent.value), "%d", readCurrentSensor());
        snprintf(panelThrust.value, sizeof(panelThrust.value), "%d", readWeightSensor());

        voltsPerPointVoltageBox.value = String(settings.getVoltsPerPointVoltage(), 4);
        voltageOffsetBox.value = String(settings.getVoltageOffset(), 2);
        voltsPerPointCurrentBox.value = String(settings.getVoltsPerPointCurrent(), 4);
        currentOffsetBox.value = String(settings.getCurrentOffset(), 2);
        thrustOffsetBox.value = String(settings.getThrustOffset(), 1);

        updatePanel(panelVoltage);
        updatePanel(panelCurrent);
        updatePanel(panelThrust);
        drawTextBox(voltsPerPointVoltageBox);
        drawTextBox(voltageOffsetBox);
        drawTextBox(voltsPerPointCurrentBox);
        drawTextBox(currentOffsetBox);
        drawTextBox(thrustOffsetBox);     }

    if (digitalRead(TOUCH_IRQ) == LOW && ts.touched()) {
        TS_Point p = ts.getPoint();
        int x = map(p.y, 0, 4095, 0, tft.width());
        int y = map(p.x, 0, 4095, 0, tft.height());

        if (x >= resetButton.x && x <= resetButton.x + resetButton.width &&
            y >= resetButton.y && y <= resetButton.y + resetButton.height) {
            rebootESP();
        }
    }
}

void TftCalibrate::rebootESP() {
    Serial.println("Reset button pressed. Rebooting...");
    ESP.restart();
}
