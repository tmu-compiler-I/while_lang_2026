#ifndef VIRTUAL_STACK_H
#define VIRTUAL_STACK_H

#include "syntax.h"

typedef enum {
    I_PUSH,
    I_RVALUE,
    I_LPUSH,
    I_PLUS,
    I_MINUS,
    I_TIMES,
    I_DIVIDE,
    I_EQ,
    I_LT,
    I_LE,
    I_GT,
    I_GE,
    I_NOT,
    I_AND,
    I_OR,
    I_TRUE,
    I_FALSE,
    I_PRINT,
    I_LABEL_TEST,
    I_LABEL_OUT,
    I_GOTO,
    I_GOFALSE,
    I_IF_START,
    I_ELSE,
    I_IF_END
} InstrKind;

typedef struct {
    InstrKind kind;
    int n;
    char *name;
    char *test;
    char *out;
} Instr;

typedef struct {
    Instr *data;
    int n, cap;
} Code;

void code_push(Code *c, Instr ins);
void code_concat(Code *dst, Code src);

Instr instr_push(int n);
Instr instr_rvalue(const char *name);
Instr instr_lpush(const char *name);
Instr instr_op(InstrKind k);
Instr instr_label_test(const char *test, const char *out);
Instr instr_label_out(const char *test, const char *out);
Instr instr_goto(const char *label);
Instr instr_gofalse(const char *label);

char *gen_label(void);
void reset_labels(void);

Code compile_arith(Arith *a);
Code compile_predicate(Pred *p);
Code compile_statement(Stmt *s);
Code compile_stack(Stmt *s);

char *format_instr(Instr ins);
char *format_code(Code code);

#endif
