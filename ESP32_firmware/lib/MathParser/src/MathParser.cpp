#include "MathParser.h"

#define TE_CONSTANT   0
#define TE_BOUND_VAR  1
#define TOK_NULL      20
#define TOK_ERROR     21
#define TOK_END       22
#define TOK_SEP       23
#define TOK_OPEN      24
#define TOK_CLOSE     25
#define TOK_NUMBER    26
#define TOK_VARIABLE  27
#define TOK_INFIX     28

#define TYPE_MASK(t)  ((t) & 0x1F)
#define ARITY(t)      (TYPE_MASK(t) < TE_FUNCTION0 ? 0 : TYPE_MASK(t) - TE_FUNCTION0)

/* ── built-in maths wrappers (needed for function-pointer binding) ─── */
static double b_add (double a,double b){return a+b;}
static double b_sub (double a,double b){return a-b;}
static double b_mul (double a,double b){return a*b;}
static double b_div (double a,double b){return a/b;}
static double b_mod (double a,double b){return fmod(a,b);}
static double b_pow (double a,double b){return pow(a,b);}
static double b_neg (double a)         {return -a;}
static double b_abs (double a)         {return fabs(a);}
static double b_ceil(double a)         {return ceil(a);}
static double b_flor(double a)         {return floor(a);}
static double b_rnd (double a)         {return round(a);}
static double b_sqrt(double a)         {return sqrt(a);}
static double b_cbrt(double a)         {return cbrt(a);}
static double b_exp (double a)         {return exp(a);}
static double b_log (double a)         {return log(a);}
static double b_lg2 (double a)         {return log2(a);}
static double b_lg10(double a)         {return log10(a);}
static double b_sin (double a)         {return sin(a);}
static double b_cos (double a)         {return cos(a);}
static double b_tan (double a)         {return tan(a);}
static double b_asin(double a)         {return asin(a);}
static double b_acos(double a)         {return acos(a);}
static double b_atan(double a)         {return atan(a);}
static double b_sinh(double a)         {return sinh(a);}
static double b_cosh(double a)         {return cosh(a);}
static double b_tanh(double a)         {return tanh(a);}
static double b_at2 (double a,double b){return atan2(a,b);}
static double b_pi  ()                 {return 3.14159265358979323846;}
static double b_e   ()                 {return 2.71828182845904523536;}

static const te_variable BUILTINS[] = {
    {"abs",   (const void*)b_abs,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"acos",  (const void*)b_acos, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"asin",  (const void*)b_asin, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"atan",  (const void*)b_atan, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"atan2", (const void*)b_at2,  TE_FUNCTION2|TE_FLAG_PURE,0},
    {"cbrt",  (const void*)b_cbrt, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"ceil",  (const void*)b_ceil, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"cos",   (const void*)b_cos,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"cosh",  (const void*)b_cosh, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"e",     (const void*)b_e,    TE_FUNCTION0|TE_FLAG_PURE,0},
    {"exp",   (const void*)b_exp,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"floor", (const void*)b_flor, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"fmod",  (const void*)b_mod,  TE_FUNCTION2|TE_FLAG_PURE,0},
    {"log",   (const void*)b_log,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"log10", (const void*)b_lg10, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"log2",  (const void*)b_lg2,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"pi",    (const void*)b_pi,   TE_FUNCTION0|TE_FLAG_PURE,0},
    {"pow",   (const void*)b_pow,  TE_FUNCTION2|TE_FLAG_PURE,0},
    {"round", (const void*)b_rnd,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"sin",   (const void*)b_sin,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"sinh",  (const void*)b_sinh, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"sqrt",  (const void*)b_sqrt, TE_FUNCTION1|TE_FLAG_PURE,0},
    {"tan",   (const void*)b_tan,  TE_FUNCTION1|TE_FLAG_PURE,0},
    {"tanh",  (const void*)b_tanh, TE_FUNCTION1|TE_FLAG_PURE,0},
    {0,0,0,0}
};

/* ── allocator ─────────────────────────────────────────────────────── */
static te_expr* new_expr(int type, te_expr* p0, te_expr* p1, te_expr* p2) {
    te_expr* n = (te_expr*)malloc(sizeof(te_expr));
    if (!n) return NULL;
    n->type = type;
    n->value = 0;
    n->parameters[0] = p0;
    n->parameters[1] = p1;
    n->parameters[2] = p2;
    return n;
}

void te_free(te_expr* n) {
    if (!n) return;
    int a = ARITY(n->type);
    if (a >= 1) te_free((te_expr*)n->parameters[0]);
    if (a >= 2) te_free((te_expr*)n->parameters[1]);
    if (a >= 3) te_free((te_expr*)n->parameters[2]);
    free(n);
}

/* ── lookup helpers ────────────────────────────────────────────────── */
static const te_variable* find_var(const te_variable* list, int len,
                                    const char* name, int nlen) {
    for (int i = 0; i < len; ++i)
        if ((int)strlen(list[i].name) == nlen &&
            strncmp(list[i].name, name, nlen) == 0)
            return &list[i];
    return NULL;
}
static const te_variable* find_builtin(const char* name, int nlen) {
    for (int i = 0; BUILTINS[i].name; ++i)
        if ((int)strlen(BUILTINS[i].name) == nlen &&
            strncmp(BUILTINS[i].name, name, nlen) == 0)
            return &BUILTINS[i];
    return NULL;
}

/* ── parser state ──────────────────────────────────────────────────── */
typedef struct {
    const char*        next;
    const char*        start;
    int                type;
    double             value;
    const void*        fn;       /* infix function pointer */
    const te_variable* bound;
    const te_variable* lookup;
    int                lookup_len;
} State;

static void next_tok(State* s) {
    s->type = TOK_NULL;
    for (;;) {
        if (!*s->next) { s->type = TOK_END; return; }
        if (isspace((unsigned char)*s->next)) { ++s->next; continue; }

        if (isdigit((unsigned char)*s->next) || *s->next == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = TOK_NUMBER; return;
        }
        if (isalpha((unsigned char)*s->next) || *s->next == '_') {
            const char* b = s->next;
            while (isalnum((unsigned char)*s->next) || *s->next == '_') ++s->next;
            int len = (int)(s->next - b);
            const te_variable* v = find_var(s->lookup, s->lookup_len, b, len);
            if (!v) v = find_builtin(b, len);
            if (!v) { s->type = TOK_ERROR; return; }
            s->bound = v;
            s->type  = TYPE_MASK(v->type) == TE_VARIABLE ? TOK_VARIABLE : v->type;
            return;
        }
        switch (*s->next) {
            case '+': s->type=TOK_INFIX; s->fn=(const void*)b_add; break;
            case '-': s->type=TOK_INFIX; s->fn=(const void*)b_sub; break;
            case '*': s->type=TOK_INFIX; s->fn=(const void*)b_mul; break;
            case '/': s->type=TOK_INFIX; s->fn=(const void*)b_div; break;
            case '%': s->type=TOK_INFIX; s->fn=(const void*)b_mod; break;
            case '^': s->type=TOK_INFIX; s->fn=(const void*)b_pow; break;
            case '(': s->type=TOK_OPEN;  break;
            case ')': s->type=TOK_CLOSE; break;
            case ',': s->type=TOK_SEP;   break;
            default:  s->type=TOK_ERROR; break;
        }
        ++s->next; return;
    }
}

/* ── recursive-descent parser ─────────────────────────────────────── */
static te_expr* te_expr_parse(State* s);
static te_expr* te_power(State* s);

static te_expr* te_base(State* s) {
    te_expr* ret = NULL;
    int t = s->type;

    if (t == TOK_NUMBER) {
        ret = new_expr(TE_CONSTANT, 0,0,0);
        if (ret) ret->value = s->value;
        next_tok(s);

    } else if (t == TOK_VARIABLE) {
        ret = new_expr(TE_BOUND_VAR, 0,0,0);
        if (ret) ret->bound = (const double*)s->bound->address;
        next_tok(s);

    } else if (TYPE_MASK(t) == TE_FUNCTION0) {
        ret = new_expr(t, 0,0,0);
        if (ret) ret->function = s->bound->address;
        next_tok(s);
        if (s->type == TOK_OPEN) {
            next_tok(s);
            if (s->type != TOK_CLOSE) s->type = TOK_ERROR;
            else next_tok(s);
        }

    } else if (TYPE_MASK(t) == TE_FUNCTION1) {
        const void* fn = s->bound->address;
        next_tok(s);
        if (s->type != TOK_OPEN) { s->type = TOK_ERROR; return NULL; }
        next_tok(s);
        te_expr* p = te_expr_parse(s);
        ret = new_expr(t, p, 0, 0);
        if (ret) ret->function = fn;
        if (s->type != TOK_CLOSE) s->type = TOK_ERROR;
        else next_tok(s);

    } else if (TYPE_MASK(t) == TE_FUNCTION2) {
        const void* fn = s->bound->address;
        next_tok(s);
        if (s->type != TOK_OPEN) { s->type = TOK_ERROR; return NULL; }
        next_tok(s);
        te_expr* p0 = te_expr_parse(s);
        if (s->type != TOK_SEP) { s->type = TOK_ERROR; te_free(p0); return NULL; }
        next_tok(s);
        te_expr* p1 = te_expr_parse(s);
        ret = new_expr(t, p0, p1, 0);
        if (ret) ret->function = fn;
        if (s->type != TOK_CLOSE) s->type = TOK_ERROR;
        else next_tok(s);

    } else if (TYPE_MASK(t) == TE_FUNCTION3) {
        const void* fn = s->bound->address;
        next_tok(s);
        if (s->type != TOK_OPEN) { s->type = TOK_ERROR; return NULL; }
        next_tok(s);
        te_expr* p0 = te_expr_parse(s);
        if (s->type != TOK_SEP) { s->type = TOK_ERROR; te_free(p0); return NULL; }
        next_tok(s);
        te_expr* p1 = te_expr_parse(s);
        if (s->type != TOK_SEP) { s->type = TOK_ERROR; te_free(p0); te_free(p1); return NULL; }
        next_tok(s);
        te_expr* p2 = te_expr_parse(s);
        ret = new_expr(t, p0, p1, p2);
        if (ret) ret->function = fn;
        if (s->type != TOK_CLOSE) s->type = TOK_ERROR;
        else next_tok(s);

    } else if (t == TOK_OPEN) {
        next_tok(s);
        ret = te_expr_parse(s);
        if (s->type != TOK_CLOSE) s->type = TOK_ERROR;
        else next_tok(s);

    } else {
        s->type = TOK_ERROR;
    }
    return ret;
}

static te_expr* te_power(State* s) {
    /* handle unary minus/plus */
    int sign = 1;
    while (s->type == TOK_INFIX &&
           (s->fn == (const void*)b_add || s->fn == (const void*)b_sub)) {
        if (s->fn == (const void*)b_sub) sign = -sign;
        next_tok(s);
    }
    te_expr* ret = te_base(s);
    /* right-assoc ^ */
    if (s->type == TOK_INFIX && s->fn == (const void*)b_pow) {
        const void* fn = s->fn;
        next_tok(s);
        te_expr* rhs = te_power(s);
        te_expr* node = new_expr(TE_FUNCTION2|TE_FLAG_PURE, ret, rhs, 0);
        if (node) node->function = fn;
        ret = node;
    }
    if (sign < 0) {
        te_expr* neg = new_expr(TE_FUNCTION1|TE_FLAG_PURE, ret, 0, 0);
        if (neg) neg->function = (const void*)b_neg;
        ret = neg;
    }
    return ret;
}

static te_expr* te_term(State* s) {
    te_expr* ret = te_power(s);
    while (s->type == TOK_INFIX &&
           (s->fn == (const void*)b_mul ||
            s->fn == (const void*)b_div ||
            s->fn == (const void*)b_mod)) {
        const void* fn = s->fn;
        next_tok(s);
        te_expr* rhs = te_power(s);
        te_expr* node = new_expr(TE_FUNCTION2|TE_FLAG_PURE, ret, rhs, 0);
        if (node) node->function = fn;
        ret = node;
    }
    return ret;
}

static te_expr* te_expr_parse(State* s) {
    te_expr* ret = te_term(s);
    while (s->type == TOK_INFIX &&
           (s->fn == (const void*)b_add ||
            s->fn == (const void*)b_sub)) {
        const void* fn = s->fn;
        next_tok(s);
        te_expr* rhs = te_term(s);
        te_expr* node = new_expr(TE_FUNCTION2|TE_FLAG_PURE, ret, rhs, 0);
        if (node) node->function = fn;
        ret = node;
    }
    return ret;
}

/* ── evaluator ─────────────────────────────────────────────────────── */
double te_eval(const te_expr* n) {
    if (!n) return NAN;
    typedef double (*f0)();
    typedef double (*f1)(double);
    typedef double (*f2)(double,double);
    typedef double (*f3)(double,double,double);
    switch (TYPE_MASK(n->type)) {
        case TE_CONSTANT:  return n->value;
        case TE_BOUND_VAR: return *n->bound;
        case TE_FUNCTION0: return ((f0)n->function)();
        case TE_FUNCTION1: return ((f1)n->function)(
            te_eval((te_expr*)n->parameters[0]));
        case TE_FUNCTION2: return ((f2)n->function)(
            te_eval((te_expr*)n->parameters[0]),
            te_eval((te_expr*)n->parameters[1]));
        case TE_FUNCTION3: return ((f3)n->function)(
            te_eval((te_expr*)n->parameters[0]),
            te_eval((te_expr*)n->parameters[1]),
            te_eval((te_expr*)n->parameters[2]));
        default: return NAN;
    }
}

/* ── public API ────────────────────────────────────────────────────── */
te_expr* te_compile(const char* expression,
                    const te_variable* variables, int var_count,
                    int* error) {
    State s;
    s.start       = expression;
    s.next        = expression;
    s.lookup      = variables;
    s.lookup_len  = var_count ? var_count : 0;
    s.type        = TOK_NULL;
    s.bound       = NULL;
    s.value       = 0;
    s.fn          = NULL;

    next_tok(&s);
    te_expr* root = te_expr_parse(&s);

    if (s.type != TOK_END) {
        te_free(root);
        if (error) *error = (int)(s.next - s.start);
        return NULL;
    }
    if (error) *error = 0;
    return root;
}

double te_interp(const char* expression, int* error) {
    te_expr* n = te_compile(expression, NULL, 0, error);
    double   r = te_eval(n);
    te_free(n);
    return r;
}
