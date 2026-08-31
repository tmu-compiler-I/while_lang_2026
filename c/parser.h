#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "syntax.h"

Stmt *parse(Tokens tokens);
Pred *parse_predicate(Tokens tokens);

#endif
