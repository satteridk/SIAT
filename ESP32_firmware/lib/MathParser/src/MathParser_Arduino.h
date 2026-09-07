#ifndef MathParser_ARDUINO_H
#define MathParser_ARDUINO_H

#include "MathParser.h"

class MathParser {
public:
    static const int MAX_VARS = 8;

    MathParser() : _expr(nullptr), _error(0), _var_count(0) {}

    ~MathParser() { _free(); }

    // ── Variable binding ──────────────────────────────────────────────────

    /**
     * Bind a named variable to a pointer.
     * The variable's value is read from the pointer at eval time.
     *
     * @param name  Name as used in the expression, e.g. "x"
     * @param ptr   Pointer to a double that holds the value
     * @return true if added, false if MAX_VARS exceeded
     */
    bool setVariable(const char* name, const double* ptr) {
        if (_var_count >= MAX_VARS) return false;
        _vars[_var_count].name    = name;
        _vars[_var_count].address = (const void*)ptr;
        _vars[_var_count].type    = TE_VARIABLE;
        _vars[_var_count].context = nullptr;
        ++_var_count;
        return true;
    }

    /**
     * Bind a custom zero-argument function.
     * e.g. addFunction("rand01", myRandFn) → use "rand01()" in expression
     */
    bool addFunction0(const char* name, double (*fn)()) {
        if (_var_count >= MAX_VARS) return false;
        _vars[_var_count++] = { name, (const void*)fn, TE_FUNCTION0 | TE_FLAG_PURE, nullptr };
        return true;
    }

    /** Bind a custom one-argument function. */
    bool addFunction1(const char* name, double (*fn)(double)) {
        if (_var_count >= MAX_VARS) return false;
        _vars[_var_count++] = { name, (const void*)fn, TE_FUNCTION1 | TE_FLAG_PURE, nullptr };
        return true;
    }

    /** Bind a custom two-argument function. */
    bool addFunction2(const char* name, double (*fn)(double, double)) {
        if (_var_count >= MAX_VARS) return false;
        _vars[_var_count++] = { name, (const void*)fn, TE_FUNCTION2 | TE_FLAG_PURE, nullptr };
        return true;
    }

    // ── Compile & eval ────────────────────────────────────────────────────

    /**
     * Compile an expression string.
     * Must call before eval(). Call again if the expression changes.
     *
     * @param expression  Math expression string
     * @return true on success, false on parse error
     */
    bool compile(const char* expression) {
        _free();
        _expr = te_compile(expression, _var_count ? _vars : nullptr,
                           _var_count, &_error);
        return _expr != nullptr;
    }

    /**
     * Evaluate the compiled expression.
     * Bound variable pointers are dereferenced at eval time —
     * change the pointed-to value and call eval() again.
     *
     * @return Result as double, or NaN if not compiled
     */
    double eval() const {
        if (!_expr) return NAN;
        return te_eval(_expr);
    }

    // ── One-shot helpers ──────────────────────────────────────────────────

    /**
     * Evaluate a simple expression with no variables.
     * Convenience: no compile step needed.
     *
     * @param expression  e.g. "2^10 + sqrt(144)"
     * @param error       optional: set to error position or 0
     */
    static double evaluate(const char* expression, int* error = nullptr) {
        int err = 0;
        double r = te_interp(expression, &err);
        if (error) *error = err;
        return r;
    }

    /**
     * Evaluate with a single variable.
     * @param expression  e.g. "x * sin(x)"
     * @param varName     variable name, e.g. "x"
     * @param value       variable value
     */
    static double evaluate(const char* expression,
                           const char* varName, double value,
                           int* error = nullptr) {
        te_variable v = { varName, &value, TE_VARIABLE, nullptr };
        int err = 0;
        te_expr* e = te_compile(expression, &v, 1, &err);
        double r = te_eval(e);
        te_free(e);
        if (error) *error = err;
        return r;
    }

    // ── Error handling ────────────────────────────────────────────────────

    /** True if last compile() succeeded. */
    bool isValid() const { return _expr != nullptr; }

    /** Error position from last compile() (0 = no error). */
    int errorPosition() const { return _error; }

    /**
     * Print a human-readable error indicator to Serial.
     * Shows the expression with a ^ pointing at the error position.
     */
    void printError(const char* expression) const {
        if (_error == 0) { Serial.println(F("No error.")); return; }
        Serial.print(F("Parse error at position "));
        Serial.print(_error);
        Serial.println(F(":"));
        Serial.println(expression);
        for (int i = 0; i < _error - 1; ++i) Serial.print(' ');
        Serial.println('^');
    }

    /** Clear all compiled expression and variable bindings. */
    void reset() {
        _free();
        _var_count = 0;
        _error = 0;
    }

private:
    te_expr*    _expr;
    int         _error;
    te_variable _vars[MAX_VARS];
    int         _var_count;

    void _free() {
        if (_expr) { te_free(_expr); _expr = nullptr; }
    }

    MathParser(const MathParser&);
    MathParser& operator=(const MathParser&);
};

#endif // MathParser_ARDUINO_H
