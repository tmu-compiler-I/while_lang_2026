"""While 言語の抽象構文木 (AST)。

コンパイラはこの木を入力として仮想スタック命令へ翻訳する。
型は OCaml 版 (syntax.ml) と同じ構成にしてある。
"""

from __future__ import annotations

from dataclasses import dataclass


# --- 算術式 -----------------------------------------------------------------

@dataclass(frozen=True)
class Var:
    name: str


@dataclass(frozen=True)
class Num:
    n: int


@dataclass(frozen=True)
class Add:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class Sub:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class Mul:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class Div:
    lhs: Arith
    rhs: Arith


Arith = Var | Num | Add | Sub | Mul | Div


# --- 条件式 -----------------------------------------------------------------

@dataclass(frozen=True)
class TrueConst:
    pass


@dataclass(frozen=True)
class FalseConst:
    pass


@dataclass(frozen=True)
class Not:
    pred: Pred


@dataclass(frozen=True)
class And:
    lhs: Pred
    rhs: Pred


@dataclass(frozen=True)
class Or:
    lhs: Pred
    rhs: Pred


@dataclass(frozen=True)
class LT:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class LE:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class GT:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class GE:
    lhs: Arith
    rhs: Arith


@dataclass(frozen=True)
class EQ:
    lhs: Arith
    rhs: Arith


Pred = TrueConst | FalseConst | Not | And | Or | LT | LE | GT | GE | EQ


# --- 文 ---------------------------------------------------------------------

@dataclass(frozen=True)
class Assign:
    name: str
    expr: Arith


@dataclass(frozen=True)
class Skip:
    pass


@dataclass(frozen=True)
class Block:
    body: Stmt


@dataclass(frozen=True)
class Seq:
    first: Stmt
    second: Stmt


@dataclass(frozen=True)
class While:
    pred: Pred
    body: Stmt


@dataclass(frozen=True)
class Print:
    expr: Arith | Pred


@dataclass(frozen=True)
class If:
    pred: Pred
    then_branch: Stmt
    else_branch: Stmt


Stmt = Assign | Skip | Block | Seq | While | Print | If


def string_of_arith(a: Arith) -> str:
    match a:
        case Var(name):
            return f"Var({name})"
        case Num(n):
            return f"Num({n})"
        case Add(lhs, rhs):
            return f"Add({string_of_arith(lhs)}, {string_of_arith(rhs)})"
        case Sub(lhs, rhs):
            return f"Sub({string_of_arith(lhs)}, {string_of_arith(rhs)})"
        case Mul(lhs, rhs):
            return f"Mul({string_of_arith(lhs)}, {string_of_arith(rhs)})"
        case Div(lhs, rhs):
            return f"Div({string_of_arith(lhs)}, {string_of_arith(rhs)})"


def string_of_pred(p: Pred) -> str:
    match p:
        case TrueConst():
            return "true"
        case FalseConst():
            return "false"
        case Not(pred):
            return f"not {string_of_pred(pred)}"
        case And(lhs, rhs):
            return f"({string_of_pred(lhs)} and {string_of_pred(rhs)})"
        case Or(lhs, rhs):
            return f"({string_of_pred(lhs)} or {string_of_pred(rhs)})"
        case LT(lhs, rhs):
            return f"{string_of_arith(lhs)} < {string_of_arith(rhs)}"
        case LE(lhs, rhs):
            return f"{string_of_arith(lhs)} <= {string_of_arith(rhs)}"
        case GT(lhs, rhs):
            return f"{string_of_arith(lhs)} > {string_of_arith(rhs)}"
        case GE(lhs, rhs):
            return f"{string_of_arith(lhs)} >= {string_of_arith(rhs)}"
        case EQ(lhs, rhs):
            return f"{string_of_arith(lhs)} == {string_of_arith(rhs)}"


def string_of_stmt(s: Stmt) -> str:
    match s:
        case Assign(name, expr):
            return f"Assign({name}, {string_of_arith(expr)})"
        case Skip():
            return "Skip"
        case Block(body):
            return f"Block({string_of_stmt(body)})"
        case Seq(first, second):
            return f"{string_of_stmt(first)}; {string_of_stmt(second)}"
        case While(pred, body):
            return f"While({string_of_pred(pred)}, {string_of_stmt(body)})"
        case Print(expr):
            if isinstance(expr, (Var, Num, Add, Sub, Mul, Div)):
                return f"Print({string_of_arith(expr)})"
            return f"Print({string_of_pred(expr)})"
        case If(pred, then_branch, else_branch):
            return (
                f"If({string_of_pred(pred)}, "
                f"{string_of_stmt(then_branch)}, "
                f"{string_of_stmt(else_branch)})"
            )
