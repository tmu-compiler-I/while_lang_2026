#ifndef INTERPRET_H
#define INTERPRET_H

#include "virtual_stack.h"

typedef struct {
    int *data;
    int n;
} Ints;

/* quiet が非0なら print を stdout に出さない。trace が非0なら各命令後の状態を stderr に出す。 */
Ints interpret(Code prog, int quiet, int trace);

#endif
