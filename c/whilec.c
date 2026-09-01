#include "emit_wasm.h"
#include "lexer.h"
#include "parser.h"
#include "syntax.h"
#include "util.h"
#include "virtual_stack.h"

#include <stdio.h>
#include <string.h>

static int usage(const char *argv0) {
    fprintf(stderr, "[usage] %s [--ast|--stack] [-o out.wat] file.while\n", argv0);
    return 1;
}

int main(int argc, char **argv) {
    init_console();
    int ast = 0, stack = 0;
    const char *outpath = NULL;
    const char *srcpath = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--ast"))
            ast = 1;
        else if (!strcmp(argv[i], "--stack"))
            stack = 1;
        else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            if (++i >= argc)
                return usage(argv[0]);
            outpath = argv[i];
        } else if (argv[i][0] == '-')
            return usage(argv[0]);
        else
            srcpath = argv[i];
    }
    if (!srcpath)
        return usage(argv[0]);

    char *src = xread_file(srcpath);
    Stmt *tree = parse(tokenize(src));
    clear_todo();
    Code code = compile_stack(tree);
    const char *todo = take_todo();
    if (todo) {
        fprintf(stderr, "error: %s\n", todo);
        return 1;
    }

    if (ast) {
        printf("%s\n", string_of_stmt(tree));
        return 0;
    }
    if (stack) {
        fputs(format_code(code), stdout);
        return 0;
    }

    char *wat = emit_wat(code);
    todo = take_todo();
    if (todo) {
        fprintf(stderr, "error: %s\n", todo);
        return 1;
    }

    char auto_out[512];
    if (!outpath) {
        snprintf(auto_out, sizeof auto_out, "%s", srcpath);
        char *dot = strrchr(auto_out, '.');
        if (dot)
            strcpy(dot, ".wat");
        else
            strcat(auto_out, ".wat");
        outpath = auto_out;
    }
    FILE *f = fopen(outpath, "w");
    if (!f)
        die("cannot write %s", outpath);
    fputs(wat, f);
    fclose(f);
    printf("wrote %s\n", outpath);
    return 0;
}
