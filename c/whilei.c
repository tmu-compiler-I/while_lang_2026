#include "interpret.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"
#include "virtual_stack.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "[usage] %s file.while\n", argv[0]);
        return 1;
    }
    char *src = xread_file(argv[1]);
    Stmt *tree = parse(tokenize(src));
    clear_todo();
    Code code = compile_stack(tree);
    const char *todo = take_todo();
    if (todo) {
        fprintf(stderr, "error: %s\n", todo);
        return 1;
    }
    interpret(code, 0);
    return 0;
}
