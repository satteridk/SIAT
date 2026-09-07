/*
 * BasicEval.ino
 * MathParser Basic Expression Evaluation
 *
 * Demonstrates evaluating math expressions from strings at runtime.
 * No variables, no custom functions just plain math.
 *
 * Use cases:
 *   - Config-driven formulas stored in EEPROM or SPIFFS
 *   - User-entered expressions via Serial / touchscreen
 *   - Dynamic threshold calculations
 *
 * Open Serial Monitor at 115200 baud.
 */

#include <MathParser_Arduino.h>

struct TestCase {
    const char* expr;
    double      expected;
};

static const TestCase tests[] = {
    {"2 + 3",              5.0},
    {"10 - 4 * 2",         2.0},
    {"2^10",            1024.0},
    {"sqrt(144)",          12.0},
    {"sin(pi/2)",           1.0},
    {"log(e)",              1.0},
    {"abs(-42)",           42.0},
    {"ceil(3.2)",           4.0},
    {"floor(3.9)",          3.0},
    {"round(3.5)",          4.0},
    {"atan2(1,1)*4",  3.14159},   // π
    {"2^3^2",           512.0},   // right-associative: 2^(3^2) = 2^9
};

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println(F("\n=== MathParser — Basic Evaluation ===\n"));

    int passed = 0;
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        int err = 0;
        double result = MathParser::evaluate(tests[i].expr, &err);

        bool ok = (err == 0) && (fabs(result - tests[i].expected) < 0.001);

        Serial.print(tests[i].expr);
        Serial.print(F("  =  "));
        Serial.print(result, 5);
        Serial.print(F("   "));
        Serial.println(ok ? F("[OK]") : F("[FAIL]"));
        if (ok) ++passed;
    }

    Serial.println();
    Serial.print(passed);
    Serial.print(F(" / "));
    Serial.print(sizeof(tests)/sizeof(tests[0]));
    Serial.println(F(" tests passed"));

    // ── Error handling demo ────────────────────────────────────────────
    Serial.println(F("\n--- Error handling ---"));
    MathParser te;
    const char* badExpr = "2 + * 3";
    bool ok = te.compile(badExpr);
    if (!ok) {
        Serial.print(F("Bad expression: "));
        te.printError(badExpr);
    }
}

void loop() { delay(5000); }
