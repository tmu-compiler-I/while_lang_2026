#ifndef INTERPRET_H
#define INTERPRET_H

#include "virtual_stack.h"

typedef struct {
    int *data;
    int n;
} Ints;

/* quiet が非0なら print を stdout に出さない。戻り値は出力された値。 */
Ints interpret(Code prog, int quiet);

#endif
