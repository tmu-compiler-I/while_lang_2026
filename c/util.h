#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdarg.h>

void *xmalloc(size_t n);
char *xstrdup(const char *s);
char *xread_file(const char *path);
void die(const char *fmt, ...);

/* 課題の未実装。whilec / test_day がメッセージを拾う。 */
void not_implemented(const char *msg);
const char *take_todo(void);
void clear_todo(void);

typedef struct {
    char *s;
    int n, cap;
} Buf;

void buf_init(Buf *b);
void buf_printf(Buf *b, const char *fmt, ...);
char *buf_take(Buf *b);

const char *test_path(const char *name);

#endif
