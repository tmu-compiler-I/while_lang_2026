#include "parser.h"
#include "util.h"

typedef struct {
    Tokens tokens;
    int i;
} Parser;

static Token peek(Parser *p) { return p->tokens.data[p->i]; }

static int accept(Parser *p, TokKind k) {
    if (peek(p).kind == k) {
        p->i++;
        return 1;
    }
    return 0;
}

static Token expect(Parser *p, TokKind k) {
    Token t = peek(p);
    if (t.kind != k)
        die("%d:%d: %s が必要ですが %s が来ました", t.line, t.col, tok_name(k),
            tok_name(t.kind));
    p->i++;
    return t;
}

static Arith *parse_arith(Parser *p);
static Pred *parse_pred(Parser *p);
static Stmt *parse_stmt(Parser *p);
static Stmt *parse_stmts(Parser *p);

static Arith *parse_atom(Parser *p) {
    Token t = peek(p);
    if (accept(p, T_NUMBER))
        return arith_num(t.num);
    if (t.kind == T_IDENT) {
        p->i++;
        return arith_var(t.ident);
    }
    if (accept(p, T_LPAREN)) {
        Arith *e = parse_arith(p);
        expect(p, T_RPAREN);
        return e;
    }
    die("%d:%d: 数値か変数が必要です (%s)", t.line, t.col, tok_name(t.kind));
    return NULL;
}

static Arith *parse_term(Parser *p) {
    Arith *left = parse_atom(p);
    for (;;) {
        if (accept(p, T_TIMES))
            left = arith_bin(ARITH_MUL, left, parse_atom(p));
        else if (accept(p, T_DIVIDE))
            left = arith_bin(ARITH_DIV, left, parse_atom(p));
        else
            return left;
    }
}

static Arith *parse_arith(Parser *p) {
    Arith *left = parse_term(p);
    for (;;) {
        if (accept(p, T_PLUS))
            left = arith_bin(ARITH_ADD, left, parse_term(p));
        else if (accept(p, T_MINUS))
            left = arith_bin(ARITH_SUB, left, parse_term(p));
        else
            return left;
    }
}

static PredKind cmp_kind(TokKind k) {
    switch (k) {
    case T_LT: return PRED_LT;
    case T_GT: return PRED_GT;
    case T_LE: return PRED_LE;
    case T_GE: return PRED_GE;
    case T_EQ: return PRED_EQ;
    default: die("internal: cmp"); return PRED_LT;
    }
}

static int is_cmp(TokKind k) {
    return k == T_LT || k == T_GT || k == T_LE || k == T_GE || k == T_EQ;
}

static Pred *parse_pred_atom(Parser *p) {
    if (accept(p, T_TRUE))
        return pred_const(PRED_TRUE);
    if (accept(p, T_FALSE))
        return pred_const(PRED_FALSE);
    if (accept(p, T_LPAREN)) {
        Pred *pred = parse_pred(p);
        expect(p, T_RPAREN);
        return pred;
    }
    Arith *left = parse_arith(p);
    Token t = peek(p);
    if (!is_cmp(t.kind))
        die("%d:%d: 比較演算子が必要です", t.line, t.col);
    p->i++;
    return pred_cmp(cmp_kind(t.kind), left, parse_arith(p));
}

static Pred *parse_pred_not(Parser *p) {
    if (accept(p, T_NOT))
        return pred_not(parse_pred_not(p));
    return parse_pred_atom(p);
}

static Pred *parse_pred_and(Parser *p) {
    Pred *left = parse_pred_not(p);
    while (accept(p, T_AND))
        left = pred_log(PRED_AND, left, parse_pred_not(p));
    return left;
}

static Pred *parse_pred(Parser *p) {
    Pred *left = parse_pred_and(p);
    while (accept(p, T_OR))
        left = pred_log(PRED_OR, left, parse_pred_and(p));
    return left;
}

static Stmt *parse_print(Parser *p) {
    Token t = peek(p);
    if (t.kind == T_TRUE || t.kind == T_FALSE || t.kind == T_NOT)
        return stmt_print_pred(parse_pred(p));
    Arith *left = parse_arith(p);
    t = peek(p);
    if (is_cmp(t.kind)) {
        p->i++;
        return stmt_print_pred(pred_cmp(cmp_kind(t.kind), left, parse_arith(p)));
    }
    return stmt_print_arith(left);
}

static Stmt *parse_stmt(Parser *p) {
    Token t = peek(p);
    if (accept(p, T_SKIP))
        return stmt_skip();
    if (accept(p, T_PRINT))
        return parse_print(p);
    if (accept(p, T_BEGIN)) {
        Stmt *body = parse_stmts(p);
        expect(p, T_END);
        return stmt_block(body);
    }
    if (accept(p, T_WHILE)) {
        Pred *pred = parse_pred(p);
        expect(p, T_DO);
        return stmt_while(pred, parse_stmt(p));
    }
    if (accept(p, T_IF)) {
        Pred *pred = parse_pred(p);
        expect(p, T_THEN);
        Stmt *th = parse_stmt(p);
        expect(p, T_ELSE);
        return stmt_if(pred, th, parse_stmt(p));
    }
    if (t.kind == T_IDENT) {
        p->i++;
        expect(p, T_ASSIGN);
        return stmt_assign(t.ident, parse_arith(p));
    }
    die("%d:%d: 文が始まりません (%s)", t.line, t.col, tok_name(t.kind));
    return NULL;
}

static Stmt *parse_stmts(Parser *p) {
    Stmt *first = parse_stmt(p);
    expect(p, T_SEMICOLON);
    TokKind k = peek(p).kind;
    if (k == T_EOF || k == T_END)
        return first;
    return stmt_seq(first, parse_stmts(p));
}

Stmt *parse(Tokens tokens) {
    Parser p = {tokens, 0};
    Stmt *s = parse_stmts(&p);
    expect(&p, T_EOF);
    return s;
}

Pred *parse_predicate(Tokens tokens) {
    Parser p = {tokens, 0};
    Pred *pred = parse_pred(&p);
    expect(&p, T_EOF);
    return pred;
}
