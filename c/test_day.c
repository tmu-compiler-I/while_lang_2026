#include "emit_wasm.h"
#include "interpret.h"
#include "lexer.h"
#include "parser.h"
#include "syntax.h"
#include "util.h"
#include "virtual_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed, failed;

static void ok(const char *name) {
    printf("  OK   %s\n", name);
    passed++;
}

static void todo_fail(const char *name, const char *msg) {
    printf("  TODO %s\n       %s\n", name, msg);
    failed++;
}

static void fail(const char *name, const char *msg) {
    printf("  FAIL %s: %s\n", name, msg);
    failed++;
}

static int contains(const char *s, const char *pat) { return strstr(s, pat) != NULL; }

static Stmt *parse_str(const char *s) { return parse(tokenize(s)); }

static Pred *parse_pred_str(const char *s) { return parse_predicate(tokenize(s)); }

static Ints compile_run_src(const char *src, int want_wat) {
    Ints empty = {0};
    clear_todo();
    Code code = compile_stack(parse_str(src));
    const char *todo = take_todo();
    if (todo) {
        empty.n = -1;
        empty.data = (int *)todo; /* 借用 */
        return empty;
    }
    Ints out = interpret(code, 1);
    if (want_wat) {
        emit_wat(code);
        todo = take_todo();
        if (todo) {
            empty.n = -1;
            empty.data = (int *)todo;
            return empty;
        }
    }
    return out;
}

static int eq_ints(Ints got, const int *want, int n) {
    if (got.n != n)
        return 0;
    for (int i = 0; i < n; i++)
        if (got.data[i] != want[i])
            return 0;
    return 1;
}

static void check_parse(const char *name, const char *src, const char *pat) {
    char *text = string_of_stmt(parse_str(src));
    if (contains(text, pat))
        ok(name);
    else
        fail(name, text);
}

static void check_pred(const char *name, const char *src, const char *pat) {
    char *text = string_of_pred(parse_pred_str(src));
    if (contains(text, pat))
        ok(name);
    else
        fail(name, text);
}

static void check_run_file(const char *name, const char *fname, const int *want, int n,
                           int want_wat) {
    const char *path = test_path(fname);
    char *src = NULL;
    FILE *f = fopen(path, "rb");
    if (!f) {
        fail(name, path);
        return;
    }
    fclose(f);
    src = xread_file(path);
    Ints out = compile_run_src(src, want_wat);
    if (out.n < 0)
        todo_fail(name, (const char *)out.data);
    else if (eq_ints(out, want, n))
        ok(name);
    else {
        Buf b;
        buf_init(&b);
        buf_printf(&b, "got");
        for (int i = 0; i < out.n; i++)
            buf_printf(&b, " %d", out.data[i]);
        fail(name, buf_take(&b));
    }
}

int main(void) {
    printf("[構文解析]\n");
    check_parse("代入 i := i + 1", "i := i + 1;", "Add");
    check_parse("引き算 i := i - 1", "i := i - 1;", "Sub");
    check_parse("かけ算 i := i * 1", "i := i * 1;", "Mul");
    check_pred("比較 i < 10", "i < 10", "<");
    check_pred("比較 i > 10", "i > 10", ">");
    check_parse("while", "while i < 10 do i := i + 1;", "While");
    check_parse("begin-end", "begin i := 1; j := 2; end;", "Block");

    printf("[課題1 算術演算]\n");
    {
        int want[] = {7, 8, 4, 23};
        check_run_file("test/arith.while → 7 8 4 23", "arith.while", want, 4, 1);
    }

    printf("[課題2 比較演算]\n");
    {
        int want[] = {1, 1, 0, 1, 1, 0};
        check_run_file("test/cmp.while → 1 1 0 1 1 0", "cmp.while", want, 6, 1);
    }

    printf("[課題3 while / begin-end]\n");
    {
        int want[] = {3};
        check_run_file("test/assign.while → 3", "assign.while", want, 1, 1);
        int loop[] = {10};
        check_run_file("test/simple_loop.while → 10", "simple_loop.while", loop, 1, 0);
        int nested[102];
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                nested[i * 10 + j] = j + 1;
        nested[100] = 10;
        nested[101] = 10;
        check_run_file("test/loop.while ネスト", "loop.while", nested, 102, 0);
        int fact[] = {120};
        check_run_file("test/fact.while → 120", "fact.while", fact, 1, 0);
    }

    printf("[発展 if]\n");
    {
        int want[] = {1};
        check_run_file("test/ifstmt.while → 1", "ifstmt.while", want, 1, 0);
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
