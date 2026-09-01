#!/usr/bin/env python3
"""WhileLang の進捗確認。

    python test_whilelang.py            # 学生提出コードを検査
    python test_whilelang.py --answer  # 講師用: answer/ を検査
"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TEST = ROOT.parent / "test"


def load_modules(use_answer: bool):
    sys.path.insert(0, str(ROOT))
    if use_answer:
        sys.path.insert(0, str(ROOT / "answer"))
    from lexer import tokenize
    from parser import parse, parse_predicate
    from syntax import string_of_pred, string_of_stmt
    from virtual_stack import compile_stack
    from emit_wasm import emit_wat
    from interpret import interpret

    return (
        tokenize,
        parse,
        parse_predicate,
        string_of_pred,
        string_of_stmt,
        compile_stack,
        emit_wat,
        interpret,
    )


def compile_run(tokenize, parse, compile_stack, interpret, src: str) -> list[int]:
    return interpret(compile_stack(parse(tokenize(src))), quiet=True)


def main() -> int:
    use_answer = "--answer" in sys.argv
    (
        tokenize,
        parse,
        parse_predicate,
        string_of_pred,
        string_of_stmt,
        compile_stack,
        emit_wat,
        interpret,
    ) = load_modules(use_answer)

    passed = 0
    failed = 0

    def check(name: str, fn):
        nonlocal passed, failed
        try:
            fn()
            print(f"  OK   {name}")
            passed += 1
        except NotImplementedError as e:
            print(f"  TODO {name}")
            print(f"       {e}")
            failed += 1
        except Exception as e:
            print(f"  FAIL {name}: {type(e).__name__}: {e}")
            failed += 1

    print("[構文解析]")

    def t_assign():
        ast = parse(tokenize("i := i + 1;"))
        text = string_of_stmt(ast)
        assert "Assign" in text and "Add" in text, text

    def t_sub_parse():
        ast = parse(tokenize("i := i - 1;"))
        assert "Sub" in string_of_stmt(ast)

    def t_mul_parse():
        ast = parse(tokenize("i := i * 1;"))
        assert "Mul" in string_of_stmt(ast)

    def t_pred_lt():
        p = parse_predicate(tokenize("i < 10"))
        assert "<" in string_of_pred(p)

    def t_pred_gt():
        p = parse_predicate(tokenize("i > 10"))
        assert ">" in string_of_pred(p)

    def t_while_parse():
        ast = parse(tokenize("while i < 10 do i := i + 1;"))
        assert "While" in string_of_stmt(ast)

    def t_block_parse():
        ast = parse(tokenize("begin i := 1; j := 2; end;"))
        text = string_of_stmt(ast)
        assert "Block" in text, text

    for name, fn in [
        ("代入 i := i + 1", t_assign),
        ("引き算 i := i - 1", t_sub_parse),
        ("かけ算 i := i * 1", t_mul_parse),
        ("比較 i < 10", t_pred_lt),
        ("比較 i > 10", t_pred_gt),
        ("while", t_while_parse),
        ("begin-end", t_block_parse),
    ]:
        check(name, fn)

    print("[課題1 算術演算]")

    def t_arith():
        src = Path(TEST / "arith.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [7, 8, 4, 23], out
        emit_wat(compile_stack(parse(tokenize(src))))

    check("test/arith.while → 7 8 4 23", t_arith)

    print("[課題2 比較演算]")

    def t_cmp():
        src = Path(TEST / "cmp.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [1, 1, 0, 1, 1, 0], out
        emit_wat(compile_stack(parse(tokenize(src))))

    check("test/cmp.while → 1 1 0 1 1 0", t_cmp)

    print("[課題3 while / begin-end]")

    def t_loop():
        src = Path(TEST / "simple_loop.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [10], out

    def t_nested():
        src = Path(TEST / "loop.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == list(range(1, 11)) * 10 + [10, 10], out

    def t_fact():
        src = Path(TEST / "fact.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [120], out

    def t_assign_run():
        src = Path(TEST / "assign.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [3], out
        wat = emit_wat(compile_stack(parse(tokenize(src))))
        assert "i32.add" in wat
        assert 'export "main"' in wat

    check("test/assign.while → 3", t_assign_run)
    check("test/simple_loop.while → 10", t_loop)
    check("test/loop.while ネスト", t_nested)
    check("test/fact.while → 120", t_fact)

    print("[発展 if]")

    def t_if():
        src = Path(TEST / "ifstmt.while").read_text(encoding="utf-8")
        out = compile_run(tokenize, parse, compile_stack, interpret, src)
        assert out == [1], out

    check("test/ifstmt.while → 1", t_if)

    print()
    print(f"{passed} passed, {failed} failed")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
