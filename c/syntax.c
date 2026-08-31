#include "syntax.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

Arith *arith_var(char *name) {
    Arith *a = xmalloc(sizeof *a);
    a->kind = ARITH_VAR;
    a->name = name;
    return a;
}

Arith *arith_num(int n) {
    Arith *a = xmalloc(sizeof *a);
    a->kind = ARITH_NUM;
    a->n = n;
    return a;
}

Arith *arith_bin(ArithKind k, Arith *lhs, Arith *rhs) {
    Arith *a = xmalloc(sizeof *a);
    a->kind = k;
    a->lhs = lhs;
    a->rhs = rhs;
    return a;
}

Pred *pred_const(PredKind k) {
    Pred *p = xmalloc(sizeof *p);
    p->kind = k;
    return p;
}

Pred *pred_not(Pred *inner) {
    Pred *p = xmalloc(sizeof *p);
    p->kind = PRED_NOT;
    p->lhs = inner;
    return p;
}

Pred *pred_log(PredKind k, Pred *lhs, Pred *rhs) {
    Pred *p = xmalloc(sizeof *p);
    p->kind = k;
    p->lhs = lhs;
    p->rhs = rhs;
    return p;
}

Pred *pred_cmp(PredKind k, Arith *a, Arith *b) {
    Pred *p = xmalloc(sizeof *p);
    p->kind = k;
    p->a = a;
    p->b = b;
    return p;
}

Stmt *stmt_assign(char *name, Arith *e) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_ASSIGN;
    s->name = name;
    s->arith = e;
    return s;
}

Stmt *stmt_skip(void) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_SKIP;
    return s;
}

Stmt *stmt_block(Stmt *body) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_BLOCK;
    s->first = body;
    return s;
}

Stmt *stmt_seq(Stmt *a, Stmt *b) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_SEQ;
    s->first = a;
    s->second = b;
    return s;
}

Stmt *stmt_while(Pred *p, Stmt *body) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_WHILE;
    s->pred = p;
    s->second = body;
    return s;
}

Stmt *stmt_print_arith(Arith *e) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_PRINT;
    s->arith = e;
    s->print_is_pred = 0;
    return s;
}

Stmt *stmt_print_pred(Pred *p) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_PRINT;
    s->pred = p;
    s->print_is_pred = 1;
    return s;
}

Stmt *stmt_if(Pred *p, Stmt *then_b, Stmt *else_b) {
    Stmt *s = xmalloc(sizeof *s);
    s->kind = STMT_IF;
    s->pred = p;
    s->first = then_b;
    s->second = else_b;
    return s;
}

char *string_of_arith(Arith *a) {
    Buf b;
    buf_init(&b);
    switch (a->kind) {
    case ARITH_VAR:
        buf_printf(&b, "Var(%s)", a->name);
        break;
    case ARITH_NUM:
        buf_printf(&b, "Num(%d)", a->n);
        break;
    case ARITH_ADD:
        buf_printf(&b, "Add(%s, %s)", string_of_arith(a->lhs), string_of_arith(a->rhs));
        break;
    case ARITH_SUB:
        buf_printf(&b, "Sub(%s, %s)", string_of_arith(a->lhs), string_of_arith(a->rhs));
        break;
    case ARITH_MUL:
        buf_printf(&b, "Mul(%s, %s)", string_of_arith(a->lhs), string_of_arith(a->rhs));
        break;
    case ARITH_DIV:
        buf_printf(&b, "Div(%s, %s)", string_of_arith(a->lhs), string_of_arith(a->rhs));
        break;
    }
    return buf_take(&b);
}

char *string_of_pred(Pred *p) {
    Buf b;
    buf_init(&b);
    switch (p->kind) {
    case PRED_TRUE:
        buf_printf(&b, "true");
        break;
    case PRED_FALSE:
        buf_printf(&b, "false");
        break;
    case PRED_NOT:
        buf_printf(&b, "not %s", string_of_pred(p->lhs));
        break;
    case PRED_AND:
        buf_printf(&b, "(%s and %s)", string_of_pred(p->lhs), string_of_pred(p->rhs));
        break;
    case PRED_OR:
        buf_printf(&b, "(%s or %s)", string_of_pred(p->lhs), string_of_pred(p->rhs));
        break;
    case PRED_LT:
        buf_printf(&b, "%s < %s", string_of_arith(p->a), string_of_arith(p->b));
        break;
    case PRED_LE:
        buf_printf(&b, "%s <= %s", string_of_arith(p->a), string_of_arith(p->b));
        break;
    case PRED_GT:
        buf_printf(&b, "%s > %s", string_of_arith(p->a), string_of_arith(p->b));
        break;
    case PRED_GE:
        buf_printf(&b, "%s >= %s", string_of_arith(p->a), string_of_arith(p->b));
        break;
    case PRED_EQ:
        buf_printf(&b, "%s == %s", string_of_arith(p->a), string_of_arith(p->b));
        break;
    }
    return buf_take(&b);
}

char *string_of_stmt(Stmt *s) {
    Buf b;
    buf_init(&b);
    switch (s->kind) {
    case STMT_ASSIGN:
        buf_printf(&b, "Assign(%s, %s)", s->name, string_of_arith(s->arith));
        break;
    case STMT_SKIP:
        buf_printf(&b, "Skip");
        break;
    case STMT_BLOCK:
        buf_printf(&b, "Block(%s)", string_of_stmt(s->first));
        break;
    case STMT_SEQ:
        buf_printf(&b, "%s; %s", string_of_stmt(s->first), string_of_stmt(s->second));
        break;
    case STMT_WHILE:
        buf_printf(&b, "While(%s, %s)", string_of_pred(s->pred), string_of_stmt(s->second));
        break;
    case STMT_PRINT:
        buf_printf(&b, "Print(%s)",
                   s->print_is_pred ? string_of_pred(s->pred) : string_of_arith(s->arith));
        break;
    case STMT_IF:
        buf_printf(&b, "If(%s, %s, %s)", string_of_pred(s->pred),
                   string_of_stmt(s->first), string_of_stmt(s->second));
        break;
    }
    return buf_take(&b);
}
