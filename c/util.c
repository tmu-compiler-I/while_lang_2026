#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *g_todo;

void *xmalloc(size_t n) {
    void *p = calloc(1, n ? n : 1);
    if (!p)
        die("out of memory");
    return p;
}

char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = xmalloc(n);
    memcpy(p, s, n);
    return p;
}

char *xread_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f)
        die("ファイルがありません: %s", path);
    if (fseek(f, 0, SEEK_END) != 0)
        die("read %s", path);
    long n = ftell(f);
    if (n < 0)
        die("read %s", path);
    rewind(f);
    char *s = xmalloc((size_t)n + 1);
    if (fread(s, 1, (size_t)n, f) != (size_t)n)
        die("read %s", path);
    fclose(f);
    s[n] = 0;
    return s;
}

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("error: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

void not_implemented(const char *msg) {
    if (!g_todo)
        g_todo = msg;
}

const char *take_todo(void) {
    const char *m = g_todo;
    g_todo = NULL;
    return m;
}

void clear_todo(void) {
    g_todo = NULL;
}

void buf_init(Buf *b) {
    b->s = NULL;
    b->n = b->cap = 0;
}

void buf_printf(Buf *b, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0)
        die("buf_printf");
    if (b->n + need + 1 > b->cap) {
        int cap = b->cap ? b->cap : 64;
        while (cap < b->n + need + 1)
            cap *= 2;
        char *s = realloc(b->s, (size_t)cap);
        if (!s)
            die("out of memory");
        b->s = s;
        b->cap = cap;
    }
    va_start(ap, fmt);
    vsprintf(b->s + b->n, fmt, ap);
    va_end(ap);
    b->n += need;
}

char *buf_take(Buf *b) {
    if (!b->s)
        return xstrdup("");
    char *s = b->s;
    b->s = NULL;
    b->n = b->cap = 0;
    return s;
}

const char *test_path(const char *name) {
    static char buf[512];
    snprintf(buf, sizeof buf, "../test/%s", name);
    if (access(buf, R_OK) == 0)
        return buf;
    snprintf(buf, sizeof buf, "test/%s", name);
    if (access(buf, R_OK) == 0)
        return buf;
    snprintf(buf, sizeof buf, "c/../test/%s", name);
    return buf;
}
