"""仮想スタック命令列のインタプリタ。

Wasm ランタイムが無くても、翻訳結果が正しいか確認できる。
"""

from __future__ import annotations

import sys

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
    format_instr,
)


class InterpretError(Exception):
    pass


def _i32(n: int) -> int:
    return n & 0xFFFFFFFF


def _signed(n: int) -> int:
    n = _i32(n)
    return n - 0x100000000 if n >= 0x80000000 else n


def _dump_trace(pc: int, ins: Instr, stack: list[int], env: dict[str, int]) -> None:
    op = format_instr(ins).replace("\t", " ")
    st = "[" + ", ".join(str(_signed(x)) for x in stack) + "]"
    ev = ", ".join(f"{k}={_signed(v)}" for k, v in sorted(env.items()))
    print(f"{pc:4d}  {op:<16}  stack={st}  env={{{ev}}}", file=sys.stderr, flush=True)


def interpret(prog: list[Instr], quiet: bool = False, trace: bool = False) -> list[int]:
    labels: dict[str, int] = {}
    for i, ins in enumerate(prog):
        match ins:
            case LabelTest(test, _out):
                labels[test] = i
            case LabelOut(_test, out):
                labels[out] = i
            case _:
                pass

    stack: list[int] = []
    env: dict[str, int] = {}
    printed: list[int] = []
    pc = 0

    def pop() -> int:
        if not stack:
            raise InterpretError(f"pc={pc}: スタックが空です")
        return stack.pop()

    while pc < len(prog):
        ins = prog[pc]
        if trace:
            _dump_trace(pc, ins, stack, env)
        jumped = False
        match ins:
            case Push(n):
                stack.append(_i32(n))
            case TrueOp():
                stack.append(1)
            case FalseOp():
                stack.append(0)
            case RValue(name):
                stack.append(env.get(name, 0))
            case LPush(name):
                env[name] = pop()
            case Plus():
                b, a = pop(), pop()
                stack.append(_i32(a + b))
            case Minus():
                b, a = pop(), pop()
                stack.append(_i32(a - b))
            case Times():
                b, a = pop(), pop()
                stack.append(_i32(a * b))
            case Divide():
                b, a = pop(), pop()
                if b == 0:
                    raise InterpretError("ゼロ除算")
                stack.append(_i32(int(_signed(a) / _signed(b))))
            case Eq():
                b, a = pop(), pop()
                stack.append(int(_signed(a) == _signed(b)))
            case Lt():
                b, a = pop(), pop()
                stack.append(int(_signed(a) < _signed(b)))
            case Le():
                b, a = pop(), pop()
                stack.append(int(_signed(a) <= _signed(b)))
            case Gt():
                b, a = pop(), pop()
                stack.append(int(_signed(a) > _signed(b)))
            case Ge():
                b, a = pop(), pop()
                stack.append(int(_signed(a) >= _signed(b)))
            case NotOp():
                stack.append(int(pop() == 0))
            case AndOp():
                b, a = pop(), pop()
                stack.append(a & b)
            case OrOp():
                b, a = pop(), pop()
                stack.append(a | b)
            case PrintOp():
                v = _signed(pop())
                printed.append(v)
                if not quiet:
                    print(v)
            case LabelTest() | LabelOut():
                pass
            case GoFalse(label):
                if pop() == 0:
                    pc = labels[label]
                    jumped = True
            case GoTo(label):
                pc = labels[label]
                jumped = True
            case IfStart():
                if pop() == 0:
                    # else まで飛ばす
                    depth = 1
                    j = pc + 1
                    while j < len(prog):
                        match prog[j]:
                            case IfStart():
                                depth += 1
                            case ElseOp() if depth == 1:
                                pc = j
                                break
                            case IfEnd():
                                depth -= 1
                                if depth == 0:
                                    pc = j
                                    break
                            case _:
                                pass
                        j += 1
            case ElseOp():
                depth = 1
                j = pc + 1
                while j < len(prog):
                    match prog[j]:
                        case IfStart():
                            depth += 1
                        case IfEnd():
                            depth -= 1
                            if depth == 0:
                                pc = j
                                break
                        case _:
                            pass
                    j += 1
            case IfEnd():
                pass
            case _:
                raise InterpretError(f"未対応の命令: {ins!r}")
        if not jumped:
            pc += 1

    return printed
