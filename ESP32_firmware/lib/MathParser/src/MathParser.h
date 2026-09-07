#ifndef MathParser_H
#define MathParser_H

#include <Arduino.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TE_VARIABLE   0
#define TE_FUNCTION0  8
#define TE_FUNCTION1  9
#define TE_FUNCTION2  10
#define TE_FUNCTION3  11
#define TE_FLAG_PURE  32

typedef struct {
    const char* name;
    const void* address;
    int         type;
    void*       context;
} te_variable;

typedef struct te_expr {
    int type;
    union { double value; const double* bound; const void* function; };
    void* parameters[3];
} te_expr;

double   te_interp(const char* expression, int* error);
te_expr* te_compile(const char* expression, const te_variable* vars, int var_count, int* error);
double   te_eval(const te_expr* expr);
void     te_free(te_expr* expr);

#ifdef __cplusplus
}
#endif
#endif
