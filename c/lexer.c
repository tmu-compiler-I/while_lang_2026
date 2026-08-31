#include "lexer.h"
#include "util.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char *tok_name(TokKind k) {
    switch (k) {
    case T_EOF: return "EOF";
    case T_NUMBER: return "NUMBER";
    case T_IDENT: return "IDENT";
    case T_PLUS: return "+";
    case T_MINUS: return "-";
    case T_TIMES: return "*";
    case T_DIVIDE: return "/";
    case T_EQ: return "==";
    case T_LT: return "<";
    case T_GT: return ">";
    case T_LE: return "<=";
    case T_GE: return ">=";
    case T_ASSIGN: return ":=";
    case T_SEMICOLON: return ";";
    case T_LPAREN: return "(";
    case T_RPAREN: return ")";
    case T_BEGIN: return "begin";
    case T_END: return "end";
    case T_WHILE: return "while";
    case T_DO: return "do";
    case T_SKIP: return "skip";
    case T_IF: return "if";
    case T_THEN: return "then";
    case T_ELSE: return "else";
    case T_PRINT: return "print";
    case T_TRUE: return "true";
    case T_FALSE: return "false";
    case T_NOT: return "not";
    case T_AND: return "and";
    case T_OR: return "or";
    }
    return "?";
}

static TokKind keyword(const char *s) {
    if (!strcmp(s, "begin")) return T_BEGIN;
    if (!strcmp(s, "end")) return T_END;
    if (!strcmp(s, "while")) return T_WHILE;
    if (!strcmp(s, "do")) return T_DO;
    if (!strcmp(s, "skip")) return T_SKIP;
    if (!strcmp(s, "if")) return T_IF;
    if (!strcmp(s, "then")) return T_THEN;
    if (!strcmp(s, "else")) return T_ELSE;
    if (!strcmp(s, "print")) return T_PRINT;
    if (!strcmp(s, "true")) return T_TRUE;
    if (!strcmp(s, "false")) return T_FALSE;
    if (!strcmp(s, "not")) return T_NOT;
    if (!strcmp(s, "and")) return T_AND;
    if (!strcmp(s, "or")) return T_OR;
    return T_IDENT;
}

static void add(Tokens *ts, Token t) {
    ts->data = realloc(ts->data, (size_t)(ts->n + 1) * sizeof(Token));
    if (!ts->data)
        die("out of memory");
    ts->data[ts->n++] = t;
}

Tokens tokenize(const char *src) {
    Tokens ts = {0};
    int i = 0, line = 1, col = 1;
    while (src[i]) {
        char c = src[i];
        if (c == ' ' || c == '\t' || c == '\r') {
            i++;
            col++;
            continue;
        }
        if (c == '\n') {
            i++;
            line++;
            col = 1;
            continue;
        }
        if (c == '#') {
            while (src[i] && src[i] != '\n')
                i++;
            continue;
        }
        int sl = line, sc = col;
        Token t = {.line = sl, .col = sc};
        if (src[i] == ':' && src[i + 1] == '=') {
            t.kind = T_ASSIGN;
            i += 2;
            col += 2;
        } else if (src[i] == '<' && src[i + 1] == '=') {
            t.kind = T_LE;
            i += 2;
            col += 2;
        } else if (src[i] == '>' && src[i + 1] == '=') {
            t.kind = T_GE;
            i += 2;
            col += 2;
        } else if (src[i] == '=' && src[i + 1] == '=') {
            t.kind = T_EQ;
            i += 2;
            col += 2;
        } else if (c == '<') {
            t.kind = T_LT;
            i++;
            col++;
        } else if (c == '>') {
            t.kind = T_GT;
            i++;
            col++;
        } else if (c == '+') {
            t.kind = T_PLUS;
            i++;
            col++;
        } else if (c == '-') {
            t.kind = T_MINUS;
            i++;
            col++;
        } else if (c == '*') {
            t.kind = T_TIMES;
            i++;
            col++;
        } else if (c == '/') {
            t.kind = T_DIVIDE;
            i++;
            col++;
        } else if (c == ';') {
            t.kind = T_SEMICOLON;
            i++;
            col++;
        } else if (c == '(') {
            t.kind = T_LPAREN;
            i++;
            col++;
        } else if (c == ')') {
            t.kind = T_RPAREN;
            i++;
            col++;
        } else if (isdigit((unsigned char)c)) {
            int n = 0;
            while (isdigit((unsigned char)src[i])) {
                n = n * 10 + (src[i] - '0');
                i++;
                col++;
            }
            t.kind = T_NUMBER;
            t.num = n;
        } else if (isalpha((unsigned char)c) || c == '_') {
            int start = i;
            while (isalnum((unsigned char)src[i]) || src[i] == '_') {
                i++;
                col++;
            }
            int len = i - start;
            char *id = xmalloc((size_t)len + 1);
            memcpy(id, src + start, (size_t)len);
            t.kind = keyword(id);
            if (t.kind == T_IDENT)
                t.ident = id;
        } else {
            die("%d:%d: 未知の文字 '%c'", line, col, c);
        }
        add(&ts, t);
    }
    add(&ts, (Token){.kind = T_EOF, .line = line, .col = col});
    return ts;
}
