from __future__ import annotations

from virtual_stack import (
    AndOp,
    Divide,
    ElseOp,
    Eq,
    FalseOp,
    Ge,
    GoFalse,
    GoTo,
    Gt,
    IfEnd,
    IfStart,
    Instr,
    LabelOut,
    LabelTest,
    Le,
    LPush,
    Lt,
    Minus,
    NotOp,
    OrOp,
    Plus,
    PrintOp,
    Push,
    RValue,
    Times,
    TrueOp,
)


class EmitError(Exception):
    pass


def collect_globals(prog: list[Instr]) -> list[str]:
    names: list[str] = []
    seen: set[str] = set()
    for ins in prog:
        match ins:
            case RValue(name) | LPush(name) if name not in seen:
                seen.add(name)
                names.append(name)
            case _:
                pass
    return names


def emit_instr(ins: Instr, ctrl: list[tuple[str, str]], indent: int) -> tuple[list[str], int]:
    """命令 1 つを WAT 行へ。戻り値は (行のリスト, 新しいインデント)。"""
    pad = " " * indent
    match ins:
        case Push(n):
            return [f"{pad}i32.const {n}"], indent
        case TrueOp():
            return [f"{pad}i32.const 1"], indent
        case FalseOp():
            return [f"{pad}i32.const 0"], indent
        case NotOp():
            return [f"{pad}i32.eqz"], indent
        case Plus():
            return [f"{pad}i32.add"], indent
        case Minus():
            raise NotImplementedError(
                "課題1: Minus は i32.sub を出力してください。"
            )
        case Times():
            raise NotImplementedError(
                "課題1: Times は i32.mul を出力してください。"
            )
        case Divide():
            return [f"{pad}i32.div_s"], indent
        case Eq():
            raise NotImplementedError(
                "課題2: Eq は i32.eq を出力してください。"
            )
        case Lt():
            return [f"{pad}i32.lt_s"], indent
        case Le():
            raise NotImplementedError(
                "課題2: Le は i32.le_s を出力してください。"
            )
        case Gt():
            raise NotImplementedError(
                "課題2: Gt は i32.gt_s を出力してください。"
            )
        case Ge():
            raise NotImplementedError(
                "課題2: Ge は i32.ge_s を出力してください。"
            )
        case AndOp():
            return [f"{pad}i32.and"], indent
        case OrOp():
            return [f"{pad}i32.or"], indent
        case RValue(name):
            return [f"{pad}global.get ${name}"], indent
        case LPush(name):
            return [f"{pad}global.set ${name}"], indent
        case PrintOp():
            return [f"{pad}call $print"], indent
        case LabelTest(test, out):
            ctrl.append((test, out))
            return [
                f"{pad}(block ${out}",
                f"{pad}  (loop ${test}",
            ], indent + 4
        case GoFalse(label):
            return [
                f"{pad}i32.eqz",
                f"{pad}br_if ${label}",
            ], indent
        case GoTo(label):
            return [f"{pad}br ${label}"], indent
        case LabelOut(test, out):
            if not ctrl:
                raise EmitError(
                    f"LabelOut({test}/{out}): 対応する LabelTest がありません"
                )
            actual_test, actual_out = ctrl.pop()
            if actual_test != test:
                raise EmitError(
                    f"LabelOut の不一致: 期待 {test}/{out}, 実際 {actual_test}/{actual_out}"
                )
            indent -= 4
            pad = " " * indent
            return [
                f"{pad}  ) ;; loop",
                f"{pad}) ;; block",
            ], indent
        case IfStart():
            return [f"{pad}if"], indent + 2
        case ElseOp():
            pad = " " * (indent - 2)
            return [f"{pad}else"], indent
        case IfEnd():
            indent -= 2
            pad = " " * indent
            return [f"{pad}end"], indent
        case _:
            raise EmitError(f"未対応の命令: {ins!r}")


def emit_wat(prog: list[Instr]) -> str:
    globals_ = collect_globals(prog)
    lines = [
        "(module",
        '  (import "env" "print" (func $print (param i32)))',
    ]
    for name in globals_:
        lines.append(f"  (global ${name} (mut i32) (i32.const 0))")
    lines.append("  (func $main")

    ctrl: list[tuple[str, str]] = []
    indent = 4
    for ins in prog:
        chunk, indent = emit_instr(ins, ctrl, indent)
        lines.extend(chunk)

    lines.append("  )")
    lines.append("")
    lines.append('  (export "main" (func $main))')
    lines.append(")")
    lines.append("")
    return "\n".join(lines)
