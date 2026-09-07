# MathParser for Arduino & ESP32

> Evaluate math expressions from strings at runtime   no dependencies, no heap beyond the parser itself.

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue?logo=arduino)](https://github.com/meerzafarnoohani/MathParser)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Arduino-orange)](https://github.com/meerzafarnoohani/MathParser)

---

## What is MathParser?

MathParser lets your Arduino evaluate a math expression stored as a string   at runtime. Instead of hardcoding `y = x * 1.8 + 32`, you store `"x * 1.8 + 32"` in EEPROM and evaluate it dynamically. Change the formula without reflashing.

**Real-world use cases:**
- 🌡️ Sensor calibration curves stored in EEPROM/SPIFFS
- ⚙️ Config-driven thresholds and formulas
- 🎮 User-entered expressions via Serial or touchscreen UI
- 🔧 Dynamic PID tuning formulas
- 🔄 Unit conversions defined at runtime
- 🤖 Joystick / motor input shaping with custom functions

---

## Supported syntax

| Category | Examples |
|----------|---------|
| Operators | `+ - * / ^ %` |
| Grouping | `( )` |
| Functions | `sin cos tan asin acos atan atan2 sinh cosh tanh` |
| | `sqrt cbrt pow exp log log2 log10` |
| | `abs ceil floor round fmod` |
| Constants | `pi` `e` |
| Variables | any name you bind via `setVariable()` |
| Custom functions | 0, 1, 2, or 3 argument function pointers |

---

## Installation

### Via Arduino IDE
1. Download `MathParser.zip` from [Releases](https://github.com/meerzafarnoohani/MathParser/releases)
2. **Sketch → Include Library → Add .ZIP Library...**

### Via Arduino Library Manager
Search **MathParser** in **Tools → Manage Libraries**.

### Manual
```bash
cd ~/Arduino/libraries
git clone https://github.com/meerzafarnoohani/MathParser.git
```

---

## Quick Start

```cpp
#include <MathParser_Arduino.h>
```

### One-shot evaluation (no variables)
```cpp
double r = MathParser::evaluate("2^10 + sqrt(144)");  // 1036.0
```

### One-shot with a single variable
```cpp
double r = MathParser::evaluate("x * sin(x)", "x", 1.5708);  // ≈ 1.5708
```

### Compiled expression with bound variables
```cpp
MathParser te;
double x = 0.0, y = 0.0;

te.setVariable("x", &x);
te.setVariable("y", &y);
te.compile("sqrt(x^2 + y^2)");   // compile once

x = 3.0; y = 4.0;
double dist = te.eval();          // 5.0   variables read at eval time

x = 5.0; y = 12.0;
dist = te.eval();                 // 13.0
```

### Custom functions
```cpp
double myClamp(double val, double limit) {
    return val > limit ? limit : val;
}

MathParser te;
te.addFunction2("clamp", myClamp);

double v = 0.0;
te.setVariable("v", &v);
te.compile("clamp(v * 2.5, 100)");

v = 30.0;
double out = te.eval();   // 75.0

v = 50.0;
out = te.eval();          // 100.0 (clamped)
```

---

## API Reference

### Static helpers (no object needed)
```cpp
double MathParser::evaluate(const char* expr, int* error = nullptr);
double MathParser::evaluate(const char* expr, const char* varName, double value, int* error = nullptr);
```

### Instance methods
```cpp
MathParser te;

// Bind variables (up to 8)
te.setVariable("x", &x);           // double* pointer

// Bind custom functions
te.addFunction0("myConst", fn);    // double fn()
te.addFunction1("myFn",    fn);    // double fn(double)
te.addFunction2("myFn2",   fn);    // double fn(double, double)

// Compile and evaluate
te.compile("x^2 + 1");             // returns bool
te.eval();                          // returns double

// Error reporting
te.isValid();                       // bool last compile succeeded?
te.errorPosition();                 // int  0 = no error
te.printError("x^2 + 1");          // print arrow pointing at error

// Reset
te.reset();                         // clear expression + all bindings
```

---

## Examples

| Example | Description |
|---------|-------------|
| `BasicEval` | One-shot evaluation of 12 test expressions with pass/fail |
| `WithVariables` | Sensor calibration formula with live ADC simulation |
| `CustomFunctions` | Register `clamp`, `deadband`, unit conversion, motor shaping |

---

## Memory

- Stack usage per `MathParser` instance: ~200 bytes (8 variable slots)
- Parser uses heap transiently during `compile()`, then frees it
- `eval()` uses no heap   walks the compiled expression tree

---

## Supported Platforms

| Platform | Status |
|----------|--------|
| ESP32 | ✅ Primary target |
| ESP8266 | ✅ Supported |
| Arduino Uno / Nano | ✅ Use short expressions |
| Arduino Mega | ✅ Supported |
| SAMD / RP2040 | ✅ Supported |

---

## Credits

Original MathParser by **Lewis Van Winkle**   [github.com/codeplea/MathParser](https://github.com/codeplea/MathParser) (zlib licence).
Arduino/ESP32 port and C++ wrapper by **Meer Zafarullah Noohani**   [github.com/meerzafarnoohani](https://github.com/meerzafarnoohani).

---

## License

MIT   see [LICENSE](LICENSE).
