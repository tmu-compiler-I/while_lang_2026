#include "interpret.h"
#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t i32(int n) { return (uint32_t)n; }

static int32_t to_signed(uint32_t n) { return (int32_t)n; }

static void ints_push(Ints *a, int v) {
    a->data = realloc(a->data, (size_t)(a->n + 1) * sizeof(int));
    if (!a->data)
        die("out of memory");
    a->data[a->n++] = v;
}

typedef struct {
    char *name;
    uint32_t val;
} Cell;

static uint32_t env_get(Cell *env, int nenv, const char *name) {
    for (int i = 0; i < nenv; i++)
        if (!strcmp(env[i].name, name))
            return env[i].val;
    return 0;
}

static void env_set(Cell *env, int *nenv, const char *name, uint32_t v) {
    for (int i = 0; i < *nenv; i++)
        if (!strcmp(env[i].name, name)) {
            env[i].val = v;
            return;
        }
    env[(*nenv)++] = (Cell){(char *)name, v};
}

static int find_label(Code prog, const char *name) {
    for (int i = 0; i < prog.n; i++) {
        Instr ins = prog.data[i];
        if (ins.kind == I_LABEL_TEST && !strcmp(ins.test, name))
            return i;
        if (ins.kind == I_LABEL_OUT && !strcmp(ins.out, name))
            return i;
    }
    die("label が見つかりません: %s", name);
    return -1;
}

static void dump_trace(int pc, Instr ins, uint32_t *stack, int sp, Cell *env, int nenv) {
    char *op = format_instr(ins);
    for (char *p = op; *p; p++)
        if (*p == '\t')
            *p = ' ';
    fprintf(stderr, "%4d  %-16s  stack=[", pc, op);
    for (int i = 0; i < sp; i++) {
        if (i)
            fputs(", ", stderr);
        fprintf(stderr, "%d", (int)to_signed(stack[i]));
    }
    fprintf(stderr, "]  env={");
    for (int i = 0; i < nenv; i++) {
        if (i)
            fputs(", ", stderr);
        fprintf(stderr, "%s=%d", env[i].name, (int)to_signed(env[i].val));
    }
    fprintf(stderr, "}\n");
    fflush(stderr);
}

Ints interpret(Code prog, int quiet, int trace) {
    uint32_t stack[256];
    int sp = 0;
    Cell env[64];
    int nenv = 0;
    Ints printed = {0};

#define POP() (sp ? stack[--sp] : (die("スタックが空です"), 0u))

    int pc = 0;
    while (pc < prog.n) {
        Instr ins = prog.data[pc];
        int jumped = 0;
        uint32_t a, b;
        if (trace)
            dump_trace(pc, ins, stack, sp, env, nenv);
        switch (ins.kind) {
        case I_PUSH:
            stack[sp++] = i32(ins.n);
            break;
        case I_TRUE:
            stack[sp++] = 1;
            break;
        case I_FALSE:
            stack[sp++] = 0;
            break;
        case I_RVALUE:
            stack[sp++] = env_get(env, nenv, ins.name);
            break;
        case I_LPUSH:
            env_set(env, &nenv, ins.name, POP());
            break;
        case I_PLUS:
            b = POP();
            a = POP();
            stack[sp++] = a + b;
            break;
        case I_MINUS:
            b = POP();
            a = POP();
            stack[sp++] = a - b;
            break;
        case I_TIMES:
            b = POP();
            a = POP();
            stack[sp++] = a * b;
            break;
        case I_DIVIDE:
            b = POP();
            a = POP();
            if (b == 0)
                die("ゼロ除算");
            stack[sp++] = (uint32_t)(to_signed(a) / to_signed(b));
            break;
        case I_EQ:
            b = POP();
            a = POP();
            stack[sp++] = to_signed(a) == to_signed(b);
            break;
        case I_LT:
            b = POP();
            a = POP();
            stack[sp++] = to_signed(a) < to_signed(b);
            break;
        case I_LE:
            b = POP();
            a = POP();
            stack[sp++] = to_signed(a) <= to_signed(b);
            break;
        case I_GT:
            b = POP();
            a = POP();
            stack[sp++] = to_signed(a) > to_signed(b);
            break;
        case I_GE:
            b = POP();
            a = POP();
            stack[sp++] = to_signed(a) >= to_signed(b);
            break;
        case I_NOT: {
            uint32_t v = POP();
            stack[sp++] = v == 0;
            break;
        }
        case I_AND:
            b = POP();
            a = POP();
            stack[sp++] = a & b;
            break;
        case I_OR:
            b = POP();
            a = POP();
            stack[sp++] = a | b;
            break;
        case I_PRINT: {
            int32_t v = to_signed(POP());
            ints_push(&printed, v);
            if (!quiet) {
                printf("%d\n", v);
                fflush(stdout);
            }
            break;
        }
        case I_LABEL_TEST:
        case I_LABEL_OUT:
            break;
        case I_GOFALSE:
            if (POP() == 0) {
                pc = find_label(prog, ins.name);
                jumped = 1;
            }
            break;
        case I_GOTO:
            pc = find_label(prog, ins.name);
            jumped = 1;
            break;
        case I_IF_START:
            if (POP() == 0) {
                int depth = 1;
                int j = pc + 1;
                while (j < prog.n) {
                    if (prog.data[j].kind == I_IF_START)
                        depth++;
                    else if (prog.data[j].kind == I_ELSE && depth == 1) {
                        pc = j;
                        break;
                    } else if (prog.data[j].kind == I_IF_END) {
                        depth--;
                        if (depth == 0) {
                            pc = j;
                            break;
                        }
                    }
                    j++;
                }
            }
            break;
        case I_ELSE: {
            int depth = 1;
            int j = pc + 1;
            while (j < prog.n) {
                if (prog.data[j].kind == I_IF_START)
                    depth++;
                else if (prog.data[j].kind == I_IF_END) {
                    depth--;
                    if (depth == 0) {
                        pc = j;
                        break;
                    }
                }
                j++;
            }
            break;
        }
        case I_IF_END:
            break;
        }
        if (!jumped)
            pc++;
    }
#undef POP
    return printed;
}
