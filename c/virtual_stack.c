/* 仮想スタック機械への翻訳。課題 1〜3 は not_implemented を埋めること。 */

#include "virtual_stack.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void code_push(Code *c, Instr ins) {
    if (c->n >= c->cap) {
        c->cap = c->cap ? c->cap * 2 : 8;
        c->data = realloc(c->data, (size_t)c->cap * sizeof(Instr));
        if (!c->data)
            die("out of memory");
    }
    c->data[c->n++] = ins;
}

void code_concat(Code *dst, Code src) {
    for (int i = 0; i < src.n; i++)
        code_push(dst, src.data[i]);
}

Instr instr_push(int n) { return (Instr){.kind = I_PUSH, .n = n}; }
Instr instr_rvalue(const char *name) {
    return (Instr){.kind = I_RVALUE, .name = (char *)name};
}
Instr instr_lpush(const char *name) {
    return (Instr){.kind = I_LPUSH, .name = (char *)name};
}
Instr instr_op(InstrKind k) { return (Instr){.kind = k}; }
Instr instr_label_test(const char *test, const char *out) {
    return (Instr){.kind = I_LABEL_TEST, .test = (char *)test, .out = (char *)out};
}
Instr instr_label_out(const char *test, const char *out) {
    return (Instr){.kind = I_LABEL_OUT, .test = (char *)test, .out = (char *)out};
}
Instr instr_goto(const char *label) {
    return (Instr){.kind = I_GOTO, .name = (char *)label};
}
Instr instr_gofalse(const char *label) {
    return (Instr){.kind = I_GOFALSE, .name = (char *)label};
}

static int label_count = -1;

char *gen_label(void) {
    char buf[32];
    label_count++;
    snprintf(buf, sizeof buf, "L.%d", label_count);
    return xstrdup(buf);
}

void reset_labels(void) { label_count = -1; }

static Code bin_arith(Arith *a, Instr op) {
    Code c = compile_arith(a->lhs);
    code_concat(&c, compile_arith(a->rhs));
    code_push(&c, op);
    return c;
}

static Code bin_pred_arith(Pred *p, Instr op) {
    Code c = compile_arith(p->a);
    code_concat(&c, compile_arith(p->b));
    code_push(&c, op);
    return c;
}

Code compile_arith(Arith *a) {
    Code c = {0};
    switch (a->kind) {
    case ARITH_VAR:
        code_push(&c, instr_rvalue(a->name));
        return c;
    case ARITH_NUM:
        code_push(&c, instr_push(a->n));
        return c;
    case ARITH_ADD:
        return bin_arith(a, instr_op(I_PLUS));
    case ARITH_SUB:
        not_implemented(
            "課題1: Sub を仮想スタック命令へ翻訳してください。"
            "Add の場合を参考に、最後に I_MINUS を置きます。");
        return c;
    case ARITH_MUL:
        not_implemented(
            "課題1: Mul を仮想スタック命令へ翻訳してください。"
            "Add の場合を参考に、最後に I_TIMES を置きます。");
        return c;
    case ARITH_DIV:
        not_implemented(
            "課題1: Div を仮想スタック命令へ翻訳してください。"
            "Add の場合を参考に、最後に I_DIVIDE を置きます。");
        return c;
    }
    return c;
}

Code compile_predicate(Pred *p) {
    Code c = {0};
    switch (p->kind) {
    case PRED_TRUE:
        code_push(&c, instr_op(I_TRUE));
        return c;
    case PRED_FALSE:
        code_push(&c, instr_op(I_FALSE));
        return c;
    case PRED_NOT:
        c = compile_predicate(p->lhs);
        code_push(&c, instr_op(I_NOT));
        return c;
    case PRED_AND:
        c = compile_predicate(p->lhs);
        code_concat(&c, compile_predicate(p->rhs));
        code_push(&c, instr_op(I_AND));
        return c;
    case PRED_OR:
        c = compile_predicate(p->lhs);
        code_concat(&c, compile_predicate(p->rhs));
        code_push(&c, instr_op(I_OR));
        return c;
    case PRED_LT:
        return bin_pred_arith(p, instr_op(I_LT));
    case PRED_LE:
        not_implemented(
            "課題2: LE を仮想スタック命令へ翻訳してください。"
            "LT の場合を参考に、最後に I_LE を置きます。");
        return c;
    case PRED_GT:
        not_implemented(
            "課題2: GT を仮想スタック命令へ翻訳してください。"
            "LT の場合を参考に、最後に I_GT を置きます。");
        return c;
    case PRED_GE:
        not_implemented(
            "課題2: GE を仮想スタック命令へ翻訳してください。"
            "LT の場合を参考に、最後に I_GE を置きます。");
        return c;
    case PRED_EQ:
        not_implemented(
            "課題2: EQ を仮想スタック命令へ翻訳してください。"
            "LT の場合を参考に、最後に I_EQ を置きます。");
        return c;
    }
    return c;
}

Code compile_statement(Stmt *s) {
    Code c = {0};
    switch (s->kind) {
    case STMT_ASSIGN:
        c = compile_arith(s->arith);
        code_push(&c, instr_lpush(s->name));
        return c;
    case STMT_SKIP:
        return c;
    case STMT_PRINT:
        c = s->print_is_pred ? compile_predicate(s->pred) : compile_arith(s->arith);
        code_push(&c, instr_op(I_PRINT));
        return c;
    case STMT_BLOCK:
        return compile_statement(s->first);
    case STMT_SEQ:
        c = compile_statement(s->first);
        code_concat(&c, compile_statement(s->second));
        return c;
    case STMT_WHILE:
        /* char *test = gen_label();
         * char *out = gen_label();
         * Code p = compile_predicate(s->pred);
         * Code body = compile_statement(s->second);
         * 上で作った LabelTest, p, GoFalse, body, GoTo, LabelOut を
         * どの順で並べると while になるか、README の翻訳例を見て書いてください。
         */
        not_implemented(
            "課題3: while をラベルとジャンプへ翻訳してください。\n"
            "README の while 翻訳例と、すぐ上のコメントを見てください。");
        return c;
    case STMT_IF:
        not_implemented(
            "発展課題: If を実装してください。"
            "条件のあとに I_IF_START, then 節, I_ELSE, else 節, I_IF_END です。");
        return c;
    }
    return c;
}

Code compile_stack(Stmt *s) {
    reset_labels();
    return compile_statement(s);
}

char *format_instr(Instr ins) {
    Buf b;
    buf_init(&b);
    switch (ins.kind) {
    case I_LPUSH: buf_printf(&b, "lpush\t%s", ins.name); break;
    case I_RVALUE: buf_printf(&b, "rvalue\t%s", ins.name); break;
    case I_PUSH: buf_printf(&b, "push\t%d", ins.n); break;
    case I_PLUS: buf_printf(&b, "+"); break;
    case I_MINUS: buf_printf(&b, "-"); break;
    case I_TIMES: buf_printf(&b, "*"); break;
    case I_DIVIDE: buf_printf(&b, "/"); break;
    case I_LT: buf_printf(&b, "<"); break;
    case I_LE: buf_printf(&b, "<="); break;
    case I_GT: buf_printf(&b, ">"); break;
    case I_GE: buf_printf(&b, ">="); break;
    case I_EQ: buf_printf(&b, "=="); break;
    case I_NOT: buf_printf(&b, "not"); break;
    case I_AND: buf_printf(&b, "and"); break;
    case I_OR: buf_printf(&b, "or"); break;
    case I_TRUE: buf_printf(&b, "true"); break;
    case I_FALSE: buf_printf(&b, "false"); break;
    case I_LABEL_TEST: buf_printf(&b, "label\t%s", ins.test); break;
    case I_LABEL_OUT: buf_printf(&b, "label\t%s", ins.out); break;
    case I_GOFALSE: buf_printf(&b, "gofalse\t%s", ins.name); break;
    case I_GOTO: buf_printf(&b, "goto\t%s", ins.name); break;
    case I_PRINT: buf_printf(&b, "print"); break;
    case I_IF_START: buf_printf(&b, "if"); break;
    case I_ELSE: buf_printf(&b, "else"); break;
    case I_IF_END: buf_printf(&b, "endif"); break;
    }
    return buf_take(&b);
}

char *format_code(Code code) {
    Buf b;
    buf_init(&b);
    for (int i = 0; i < code.n; i++)
        buf_printf(&b, "%s\n", format_instr(code.data[i]));
    return buf_take(&b);
}
