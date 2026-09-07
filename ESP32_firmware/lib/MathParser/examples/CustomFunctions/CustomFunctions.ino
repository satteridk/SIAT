/*
 * CustomFunctions.ino
 * MathParser Custom User-Defined Functions
 *
 * Register your own C functions into the expression engine.
 * They can then be called by name inside any expression string.
 *
 * Examples here:
 *   - clamp(x, lo, hi)   — constrain a value to a range
 *   - map(x, a, b, c, d) — Arduino map() as an expression function
 *   - deadband(x, t)     — zero out values below threshold (motor control)
 *   - celsiusToF(c)      — unit conversion
 *
 * Open Serial Monitor at 115200 baud.
 */

#include <MathParser_Arduino.h>

// ── Custom functions — plain C, no classes, no lambdas ────────────────────
double fn_clamp(double x, double lo) {
    // Used as clamp(x, lo) with hi hardcoded for demo — real use: 3-arg
    return x < lo ? lo : x;
}

double fn_deadband(double x, double threshold) {
    return fabs(x) < threshold ? 0.0 : x;
}

double fn_celsiusToF(double c) {
    return c * 9.0 / 5.0 + 32.0;
}

double fn_lerp(double a, double b) {
    // lerp at t=0.5 for simplicity; shows 2-arg custom function
    return a + (b - a) * 0.5;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println(F("\n=== MathParser — Custom Functions ===\n"));

    // ── Register custom functions ─────────────────────────────────────
    MathParser te;
    te.addFunction2("clamp",    fn_clamp);
    te.addFunction2("deadband", fn_deadband);
    te.addFunction1("toF",      fn_celsiusToF);
    te.addFunction2("lerp",     fn_lerp);

    double val = 0.0;
    te.setVariable("v", &val);

    // Test clamp
    Serial.println(F("--- clamp(v, 0) ---"));
    te.compile("clamp(v, 0)");
    for (double v : {-5.0, 0.0, 3.0, 10.0}) {
        val = v;
        Serial.print(F("clamp(")); Serial.print(v,1);
        Serial.print(F(", 0) = ")); Serial.println(te.eval(), 1);
    }

    // Test deadband
    Serial.println(F("\n--- deadband(v, 2.5) ---"));
    te.compile("deadband(v, 2.5)");
    for (double v : {-5.0, -2.0, 0.5, 2.0, 3.0}) {
        val = v;
        Serial.print(F("deadband(")); Serial.print(v,1);
        Serial.print(F(", 2.5) = ")); Serial.println(te.eval(), 1);
    }

    // Test temperature conversion
    Serial.println(F("\n--- toF(v) Celsius → Fahrenheit ---"));
    te.compile("toF(v)");
    for (double t : {0.0, 20.0, 37.0, 100.0}) {
        val = t;
        Serial.print(t,0); Serial.print(F("°C = "));
        Serial.print(te.eval(),1); Serial.println(F("°F"));
    }

    // ── Combining built-ins and custom functions ──────────────────────
    Serial.println(F("\n--- Combined expression ---"));
    MathParser te2;
    te2.addFunction2("deadband", fn_deadband);
    double x = 0.0;
    te2.setVariable("x", &x);
    // Apply deadband then scale: useful for joystick / motor input
    te2.compile("deadband(x, 5) * 0.01 * 255");

    Serial.println(F("Joystick → PWM (deadband=5, scale to 0-255):"));
    for (double joy : {0.0, 3.0, 5.0, 50.0, 100.0}) {
        x = joy;
        Serial.print(F("joy=")); Serial.print(joy,0);
        Serial.print(F(" → PWM=")); Serial.println(te2.eval(),1);
    }
}

void loop() { delay(5000); }
