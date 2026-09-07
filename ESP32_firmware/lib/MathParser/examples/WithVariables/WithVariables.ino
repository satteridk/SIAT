/*
 * WithVariables.ino
 * MathParser Expressions with Bound Variables
 *
 * Compile an expression once, change variable values, evaluate repeatedly.
 * This is the most common real-world use case:
 *
 *   - Sensor calibration formula stored in config
 *   - PID formula with live measurements
 *   - Alarm thresholds: "temp > 80 ? 1 : 0" (boolean logic)
 *   - Unit conversion: "f * 9/5 + 32" (Celsius to Fahrenheit)
 *
 * Open Serial Monitor at 115200 baud.
 */

#include <MathParser_Arduino.h>

// ── Simulated sensor values (replace with real ADC / I2C reads) ────────────
double raw_temp    = 0.0;   // raw ADC reading
double raw_voltage = 0.0;   // raw ADC reading

// ── Two separate MathParser instances for two formulas ──────────────────────
MathParser tempFormula;    // converts raw ADC to Celsius
MathParser voltFormula;    // converts raw ADC to voltage

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println(F("\n=== MathParser — With Variables ===\n"));

    // Bind variables by pointer
    tempFormula.setVariable("raw", &raw_temp);
    voltFormula.setVariable("raw", &raw_voltage);

    // Compile formulas once — these could come from EEPROM/SPIFFS in production
    // ESP32 12-bit ADC: raw 0-4095, 3.3V reference, NTC offset
    bool ok1 = tempFormula.compile("(raw / 4095.0) * 330.0 - 50.0");
    bool ok2 = voltFormula.compile("(raw / 4095.0) * 3.3");

    if (!ok1) { Serial.println(F("tempFormula compile failed!")); return; }
    if (!ok2) { Serial.println(F("voltFormula compile failed!")); return; }

    Serial.println(F("Formulas compiled successfully."));
    Serial.println(F("raw_adc\t\tTemp(C)\t\tVoltage(V)"));
    Serial.println(F("-------\t\t-------\t\t----------"));
}

void loop() {
    // Simulate ADC readings scanning 0 → 4095
    for (int adc = 0; adc <= 4095; adc += 512) {
        raw_temp    = adc;
        raw_voltage = adc;

        double temp    = tempFormula.eval();
        double voltage = voltFormula.eval();

        Serial.print(adc);
        Serial.print(F("\t\t"));
        Serial.print(temp, 1);
        Serial.print(F("\t\t"));
        Serial.println(voltage, 3);
    }

    Serial.println();

    // ── Multi-variable example ─────────────────────────────────────────
    Serial.println(F("--- Quadratic: ax^2 + bx + c ---"));
    double a = 1.0, b = -3.0, c = 2.0, x = 0.0;

    MathParser quad;
    quad.setVariable("a", &a);
    quad.setVariable("b", &b);
    quad.setVariable("c", &c);
    quad.setVariable("x", &x);
    quad.compile("a*x^2 + b*x + c");

    for (double xi = 0.0; xi <= 4.0; xi += 0.5) {
        x = xi;
        Serial.print(F("x="));  Serial.print(x, 1);
        Serial.print(F("  →  ")); Serial.println(quad.eval(), 4);
    }

    delay(10000);
}
