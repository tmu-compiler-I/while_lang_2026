#ifndef LEXER_H
#define LEXER_H

typedef enum {
    T_EOF,
    T_NUMBER,
    T_IDENT,
    T_PLUS,
    T_MINUS,
    T_TIMES,
    T_DIVIDE,
    T_EQ,
    T_LT,
    T_GT,
    T_LE,
    T_GE,
    T_ASSIGN,
    T_SEMICOLON,
    T_LPAREN,
    T_RPAREN,
    T_BEGIN,
    T_END,
    T_WHILE,
    T_DO,
    T_SKIP,
    T_IF,
    T_THEN,
    T_ELSE,
    T_PRINT,
    T_TRUE,
    T_FALSE,
    T_NOT,
    T_AND,
    T_OR
} TokKind;

typedef struct {
    TokKind kind;
    int num;
    char *ident;
    int line, col;
} Token;

typedef struct {
    Token *data;
    int n;
} Tokens;

const char *tok_name(TokKind k);
Tokens tokenize(const char *src);

#endif
