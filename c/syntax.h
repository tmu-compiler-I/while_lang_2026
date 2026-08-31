#ifndef SYNTAX_H
#define SYNTAX_H

typedef struct Arith Arith;
typedef struct Pred Pred;
typedef struct Stmt Stmt;

typedef enum {
    ARITH_VAR,
    ARITH_NUM,
    ARITH_ADD,
    ARITH_SUB,
    ARITH_MUL,
    ARITH_DIV
} ArithKind;

struct Arith {
    ArithKind kind;
    char *name;
    int n;
    Arith *lhs, *rhs;
};

typedef enum {
    PRED_TRUE,
    PRED_FALSE,
    PRED_NOT,
    PRED_AND,
    PRED_OR,
    PRED_LT,
    PRED_LE,
    PRED_GT,
    PRED_GE,
    PRED_EQ
} PredKind;

struct Pred {
    PredKind kind;
    Pred *lhs, *rhs; /* NOT は lhs だけ使う */
    Arith *a, *b;    /* 比較 */
};

typedef enum {
    STMT_ASSIGN,
    STMT_SKIP,
    STMT_BLOCK,
    STMT_SEQ,
    STMT_WHILE,
    STMT_PRINT,
    STMT_IF
} StmtKind;

struct Stmt {
    StmtKind kind;
    char *name;
    Arith *arith;
    Pred *pred;
    int print_is_pred;
    Stmt *first, *second; /* SEQ / BLOCK(first) / WHILE(second=body) / IF */
};

Arith *arith_var(char *name);
Arith *arith_num(int n);
Arith *arith_bin(ArithKind k, Arith *lhs, Arith *rhs);

Pred *pred_const(PredKind k);
Pred *pred_not(Pred *p);
Pred *pred_log(PredKind k, Pred *lhs, Pred *rhs);
Pred *pred_cmp(PredKind k, Arith *a, Arith *b);

Stmt *stmt_assign(char *name, Arith *e);
Stmt *stmt_skip(void);
Stmt *stmt_block(Stmt *body);
Stmt *stmt_seq(Stmt *a, Stmt *b);
Stmt *stmt_while(Pred *p, Stmt *body);
Stmt *stmt_print_arith(Arith *e);
Stmt *stmt_print_pred(Pred *p);
Stmt *stmt_if(Pred *p, Stmt *then_b, Stmt *else_b);

char *string_of_arith(Arith *a);
char *string_of_pred(Pred *p);
char *string_of_stmt(Stmt *s);

#endif
