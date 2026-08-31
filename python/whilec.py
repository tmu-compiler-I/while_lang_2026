#!/usr/bin/env python3
"""WHILE 言語コンパイラのエントリポイント

    python whilec.py ../test/assign.while           # .wat を出力
    python whilec.py --stack ../test/assign.while   # 仮想スタック命令を表示
    python whilec.py --run ../test/assign.while     # 仮想機械で実行
    python whilec.py --ast ../test/assign.while     # AST を表示
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from emit_wasm import emit_wat
from interpret import interpret
from lexer import tokenize
from parser import parse
from syntax import string_of_stmt
from virtual_stack import compile_stack, format_code


def compile_source(src: str):
    tokens = tokenize(src)
    ast = parse(tokens)
    return ast, compile_stack(ast)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="WHILE to WebAssembly コンパイラ")
    ap.add_argument("source", help=".while ファイル")
    ap.add_argument("--ast", action="store_true", help="構文木を表示する")
    ap.add_argument("--stack", action="store_true", help="仮想スタック命令を表示する")
    ap.add_argument("--run", action="store_true", help="仮想スタック命令を実行する")
    ap.add_argument("-o", "--output", help="出力 .wat のパス")
    args = ap.parse_args(argv)

    path = Path(args.source)
    if not path.exists():
        print(f"ファイルがありません: {path}", file=sys.stderr)
        return 1

    src = path.read_text(encoding="utf-8")
    try:
        ast, code = compile_source(src)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    if args.ast:
        print(string_of_stmt(ast))
        return 0

    if args.stack:
        print(format_code(code), end="")
        return 0

    if args.run:
        interpret(code)
        return 0

    wat = emit_wat(code)
    out = Path(args.output) if args.output else path.with_suffix(".wat")
    out.write_text(wat, encoding="utf-8")
    print(f"wrote {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
