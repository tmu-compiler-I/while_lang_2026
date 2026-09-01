"""WhileLang の AST を、スタック機械の命令列へ変換する。

課題 1〜3 はここの NotImplementedError を埋めること。
"""

from __future__ import annotations

from dataclasses import dataclass

from syntax import (
    Add,
    And,
    Arith,
    Assign,
    Block,
    Div,
    EQ,
    FalseConst,
    GE,
    GT,
    If,
    LE,
    LT,
    Mul,
    Not,
    Num,
    Or,
    Pred,
    Print,
    Seq,
    Skip,
    Stmt,
    Sub,
    TrueConst,
    Var,
    While,
)


# --- 命令 -------------------------------------------------------------------

@dataclass(frozen=True)
class Push:
    n: int


@dataclass(frozen=True)
class RValue:
    name: str


@dataclass(frozen=True)
class LPush:
    name: str


@dataclass(frozen=True)
class Plus:
    pass


@dataclass(frozen=True)
class Minus:
    pass


@dataclass(frozen=True)
class Times:
    pass


@dataclass(frozen=True)
class Divide:
    pass


@dataclass(frozen=True)
class Eq:
    pass


@dataclass(frozen=True)
class Lt:
    pass


@dataclass(frozen=True)
class Le:
    pass


@dataclass(frozen=True)
class Gt:
    pass


@dataclass(frozen=True)
class Ge:
    pass


@dataclass(frozen=True)
class NotOp:
    pass


@dataclass(frozen=True)
class AndOp:
    pass


@dataclass(frozen=True)
class OrOp:
    pass


@dataclass(frozen=True)
class TrueOp:
    pass


@dataclass(frozen=True)
class FalseOp:
    pass


@dataclass(frozen=True)
class PrintOp:
    pass


@dataclass(frozen=True)
class LabelTest:
    test: str
    out: str


@dataclass(frozen=True)
class LabelOut:
    test: str
    out: str


@dataclass(frozen=True)
class GoTo:
    label: str


@dataclass(frozen=True)
class GoFalse:
    label: str


@dataclass(frozen=True)
class IfStart:
    pass


@dataclass(frozen=True)
class ElseOp:
    pass


@dataclass(frozen=True)
class IfEnd:
    pass


Instr = (
    Push
    | RValue
    | LPush
    | Plus
    | Minus
    | Times
    | Divide
    | Eq
    | Lt
    | Le
    | Gt
    | Ge
    | NotOp
    | AndOp
    | OrOp
    | TrueOp
    | FalseOp
    | PrintOp
    | LabelTest
    | LabelOut
    | GoTo
    | GoFalse
    | IfStart
    | ElseOp
    | IfEnd
)


_count = -1


def gen_label() -> str:
    global _count
    _count += 1
    return f"L.{_count}"


def reset() -> None:
    global _count
    _count = -1


def compile_arith(arith: Arith) -> list[Instr]:
    match arith:
        case Var(name):
            return [RValue(name)]
        case Num(n):
            return [Push(n)]
        case Add(lhs, rhs):
            return compile_arith(lhs) + compile_arith(rhs) + [Plus()]
        case Sub(lhs, rhs):
            raise NotImplementedError(
                "課題1: Sub を仮想スタック命令へ翻訳してください。"
                "Add の場合を参考に、最後に Minus() を置きます。"
            )
        case Mul(lhs, rhs):
            raise NotImplementedError(
                "課題1: Mul を仮想スタック命令へ翻訳してください。"
                "Add の場合を参考に、最後に Times() を置きます。"
            )
        case Div(lhs, rhs):
            raise NotImplementedError(
                "課題1: Div を仮想スタック命令へ翻訳してください。"
                "Add の場合を参考に、最後に Divide() を置きます。"
            )


def compile_predicate(pred: Pred) -> list[Instr]:
    match pred:
        case TrueConst():
            return [TrueOp()]
        case FalseConst():
            return [FalseOp()]
        case Not(p):
            return compile_predicate(p) + [NotOp()]
        case And(lhs, rhs):
            return compile_predicate(lhs) + compile_predicate(rhs) + [AndOp()]
        case Or(lhs, rhs):
            return compile_predicate(lhs) + compile_predicate(rhs) + [OrOp()]
        case LT(lhs, rhs):
            return compile_arith(lhs) + compile_arith(rhs) + [Lt()]
        case LE(lhs, rhs):
            raise NotImplementedError(
                "課題2: LE を仮想スタック命令へ翻訳してください。"
                "LT の場合を参考に、最後に Le() を置きます。"
            )
        case GT(lhs, rhs):
            raise NotImplementedError(
                "課題2: GT を仮想スタック命令へ翻訳してください。"
                "LT の場合を参考に、最後に Gt() を置きます。"
            )
        case GE(lhs, rhs):
            raise NotImplementedError(
                "課題2: GE を仮想スタック命令へ翻訳してください。"
                "LT の場合を参考に、最後に Ge() を置きます。"
            )
        case EQ(lhs, rhs):
            raise NotImplementedError(
                "課題2: EQ を仮想スタック命令へ翻訳してください。"
                "LT の場合を参考に、最後に Eq() を置きます。"
            )


def compile_statement(stmt: Stmt) -> list[Instr]:
    match stmt:
        case Assign(name, expr):
            return compile_arith(expr) + [LPush(name)]
        case Skip():
            return []
        case Print(expr):
            if isinstance(expr, (TrueConst, FalseConst, Not, And, Or, LT, LE, GT, GE, EQ)):
                return compile_predicate(expr) + [PrintOp()]
            return compile_arith(expr) + [PrintOp()]
        case Block(body):
            return compile_statement(body)
        case Seq(first, second):
            return compile_statement(first) + compile_statement(second)
        case While(pred, body):
            # test = gen_label()
            # out = gen_label()
            # p = compile_predicate(pred)
            # s = compile_statement(body)
            # 上で作った LabelTest, p, GoFalse, s, GoTo, LabelOut を
            # どの順で並べると while になるか、README の翻訳例を見て書いてください。
            raise NotImplementedError(
                "課題3: while をラベルとジャンプへ翻訳してください。\n"
                "README の while 翻訳例と、すぐ上のコメントを見てください。"
            )
        case If(_pred, _then, _else):
            raise NotImplementedError(
                "発展課題: If を実装してください。"
                "条件のあとに IfStart(), then 節, ElseOp(), else 節, IfEnd() です。"
            )


def compile_stack(stmt: Stmt) -> list[Instr]:
    reset()
    return compile_statement(stmt)


def format_instr(ins: Instr) -> str:
    match ins:
        case LPush(name):
            return f"lpush\t{name}"
        case RValue(name):
            return f"rvalue\t{name}"
        case Push(n):
            return f"push\t{n}"
        case Plus():
            return "+"
        case Minus():
            return "-"
        case Times():
            return "*"
        case Divide():
            return "/"
        case Lt():
            return "<"
        case Le():
            return "<="
        case Gt():
            return ">"
        case Ge():
            return ">="
        case Eq():
            return "=="
        case NotOp():
            return "not"
        case AndOp():
            return "and"
        case OrOp():
            return "or"
        case TrueOp():
            return "true"
        case FalseOp():
            return "false"
        case LabelTest(test, _out):
            return f"label\t{test}"
        case LabelOut(_test, out):
            return f"label\t{out}"
        case GoFalse(label):
            return f"gofalse\t{label}"
        case GoTo(label):
            return f"goto\t{label}"
        case PrintOp():
            return "print"
        case IfStart():
            return "if"
        case ElseOp():
            return "else"
        case IfEnd():
            return "endif"


def format_code(code: list[Instr]) -> str:
    return "".join(format_instr(ins) + "\n" for ins in code)
