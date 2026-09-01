#!/usr/bin/env python3
"""仮想スタック機械で WhileLang プログラムを実行する。

    python whilei.py ../test/assign.while
    python whilei.py --trace ../test/assign.while
"""

from __future__ import annotations

import sys
from pathlib import Path

from interpret import interpret
from lexer import tokenize
from parser import parse
from virtual_stack import compile_stack


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    trace = False
    if args and args[0] == "--trace":
        trace = True
        args = args[1:]
    if len(args) != 1:
        print(f"[usage] {Path(sys.argv[0]).name} [--trace] file.while", file=sys.stderr)
        return 1
    path = Path(args[0])
    if not path.exists():
        print(f"ファイルがありません: {path}", file=sys.stderr)
        return 1
    try:
        ast = parse(tokenize(path.read_text(encoding="utf-8")))
        interpret(compile_stack(ast), trace=trace)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
