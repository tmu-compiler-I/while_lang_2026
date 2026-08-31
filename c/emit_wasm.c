/* 仮想スタック命令列 → WebAssembly Text Format (.wat)
 * 課題 1・2 では未実装の演算を埋める。while の制御構造は配布済み。
 */

#include "emit_wasm.h"
#include "util.h"

#include <string.h>

typedef struct {
    char *test;
    char *out;
} CFrame;

static int collect_globals(Code prog, char **names, int max) {
    int n = 0;
    for (int i = 0; i < prog.n; i++) {
        Instr ins = prog.data[i];
        const char *name = NULL;
        if (ins.kind == I_RVALUE || ins.kind == I_LPUSH)
            name = ins.name;
        if (!name)
            continue;
        int seen = 0;
        for (int j = 0; j < n; j++)
            if (!strcmp(names[j], name))
                seen = 1;
        if (!seen && n < max)
            names[n++] = (char *)name;
    }
    return n;
}

static void emit_instr(Buf *b, Instr ins, CFrame *ctrl, int *nctrl, int *indent) {
    int pad = *indent;
    switch (ins.kind) {
    case I_PUSH:
        buf_printf(b, "%*si32.const %d\n", pad, "", ins.n);
        break;
    case I_TRUE:
        buf_printf(b, "%*si32.const 1\n", pad, "");
        break;
    case I_FALSE:
        buf_printf(b, "%*si32.const 0\n", pad, "");
        break;
    case I_NOT:
        buf_printf(b, "%*si32.eqz\n", pad, "");
        break;
    case I_PLUS:
        buf_printf(b, "%*si32.add\n", pad, "");
        break;
    case I_MINUS:
        not_implemented("課題1: Minus は i32.sub を出力してください。");
        break;
    case I_TIMES:
        not_implemented("課題1: Times は i32.mul を出力してください。");
        break;
    case I_DIVIDE:
        buf_printf(b, "%*si32.div_s\n", pad, "");
        break;
    case I_EQ:
        not_implemented("課題2: Eq は i32.eq を出力してください。");
        break;
    case I_LT:
        buf_printf(b, "%*si32.lt_s\n", pad, "");
        break;
    case I_LE:
        not_implemented("課題2: Le は i32.le_s を出力してください。");
        break;
    case I_GT:
        not_implemented("課題2: Gt は i32.gt_s を出力してください。");
        break;
    case I_GE:
        not_implemented("課題2: Ge は i32.ge_s を出力してください。");
        break;
    case I_AND:
        buf_printf(b, "%*si32.and\n", pad, "");
        break;
    case I_OR:
        buf_printf(b, "%*si32.or\n", pad, "");
        break;
    case I_RVALUE:
        buf_printf(b, "%*sglobal.get $%s\n", pad, "", ins.name);
        break;
    case I_LPUSH:
        buf_printf(b, "%*sglobal.set $%s\n", pad, "", ins.name);
        break;
    case I_PRINT:
        buf_printf(b, "%*scall $print\n", pad, "");
        break;
    case I_LABEL_TEST:
        ctrl[(*nctrl)++] = (CFrame){ins.test, ins.out};
        buf_printf(b, "%*s(block $%s\n", pad, "", ins.out);
        buf_printf(b, "%*s  (loop $%s\n", pad, "", ins.test);
        *indent += 4;
        break;
    case I_GOFALSE:
        buf_printf(b, "%*si32.eqz\n", pad, "");
        buf_printf(b, "%*sbr_if $%s\n", pad, "", ins.name);
        break;
    case I_GOTO:
        buf_printf(b, "%*sbr $%s\n", pad, "", ins.name);
        break;
    case I_LABEL_OUT:
        if (*nctrl == 0)
            die("LabelOut(%s/%s): 対応する LabelTest がありません", ins.test, ins.out);
        {
            CFrame f = ctrl[--*nctrl];
            if (strcmp(f.test, ins.test) != 0)
                die("LabelOut の不一致: 期待 %s/%s, 実際 %s/%s", ins.test, ins.out, f.test,
                    f.out);
        }
        *indent -= 4;
        pad = *indent;
        buf_printf(b, "%*s  ) ;; loop\n", pad, "");
        buf_printf(b, "%*s) ;; block\n", pad, "");
        break;
    case I_IF_START:
        buf_printf(b, "%*sif\n", pad, "");
        *indent += 2;
        break;
    case I_ELSE:
        buf_printf(b, "%*selse\n", pad - 2, "");
        break;
    case I_IF_END:
        *indent -= 2;
        buf_printf(b, "%*send\n", *indent, "");
        break;
    }
}

char *emit_wat(Code prog) {
    char *globals[64];
    int ng = collect_globals(prog, globals, 64);
    Buf b;
    buf_init(&b);
    buf_printf(&b, "(module\n");
    buf_printf(&b, "  (import \"env\" \"print\" (func $print (param i32)))\n");
    for (int i = 0; i < ng; i++)
        buf_printf(&b, "  (global $%s (mut i32) (i32.const 0))\n", globals[i]);
    buf_printf(&b, "  (func $main\n");

    CFrame ctrl[32];
    int nctrl = 0;
    int indent = 4;
    for (int i = 0; i < prog.n; i++)
        emit_instr(&b, prog.data[i], ctrl, &nctrl, &indent);

    buf_printf(&b, "  )\n\n");
    buf_printf(&b, "  (export \"main\" (func $main))\n");
    buf_printf(&b, ")\n");
    return buf_take(&b);
}
