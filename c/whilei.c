#include "interpret.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"
#include "virtual_stack.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int trace = 0;
    const char *srcpath = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--trace"))
            trace = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "[usage] %s [--trace] file.while\n", argv[0]);
            return 1;
        } else
            srcpath = argv[i];
    }
    if (!srcpath) {
        fprintf(stderr, "[usage] %s [--trace] file.while\n", argv[0]);
        return 1;
    }
    char *src = xread_file(srcpath);
    Stmt *tree = parse(tokenize(src));
    clear_todo();
    Code code = compile_stack(tree);
    const char *todo = take_todo();
    if (todo) {
        fprintf(stderr, "error: %s\n", todo);
        return 1;
    }
    interpret(code, 0, trace);
    return 0;
}
