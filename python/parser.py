"""再帰下降パーサ。トークン列を AST へ変換する。

このファイルは配布済み (課題で変更する必要はない)。
演算子の優先順位:

  * /     が高い
  + -     がその次
  比較    がその次
  not
  and
  or      が最も低い
"""

from __future__ import annotations

from lexer import Token
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


class ParseError(Exception):
    pass


class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.i = 0

    def peek(self) -> Token:
        return self.tokens[self.i]

    def accept(self, *kinds: str) -> Token | None:
        tok = self.peek()
        if tok.kind in kinds:
            self.i += 1
            return tok
        return None

    def expect(self, kind: str) -> Token:
        tok = self.accept(kind)
        if tok is None:
            got = self.peek()
            raise ParseError(
                f"{got.line}:{got.col}: {kind} が必要ですが {got.kind} が来ました"
            )
        return tok

    def parse(self) -> Stmt:
        stmt = self.statements()
        self.expect("EOF")
        return stmt

    def parse_predicate(self) -> Pred:
        pred = self.predicate()
        self.expect("EOF")
        return pred

    def statements(self) -> Stmt:
        first = self.statement()
        self.expect("SEMICOLON")
        if self.peek().kind in ("EOF", "END"):
            return first
        return Seq(first, self.statements())

    def statement(self) -> Stmt:
        tok = self.peek()
        if self.accept("SKIP"):
            return Skip()
        if self.accept("PRINT"):
            return Print(self.print_arg())
        if self.accept("BEGIN"):
            body = self.statements()
            self.expect("END")
            return Block(body)
        if self.accept("WHILE"):
            pred = self.predicate()
            self.expect("DO")
            return While(pred, self.statement())
        if self.accept("IF"):
            pred = self.predicate()
            self.expect("THEN")
            then_branch = self.statement()
            self.expect("ELSE")
            return If(pred, then_branch, self.statement())
        if tok.kind == "IDENT":
            name = tok.value
            assert isinstance(name, str)
            self.i += 1
            self.expect("ASSIGN")
            return Assign(name, self.arith())
        raise ParseError(
            f"{tok.line}:{tok.col}: 文が始まりません ({tok.kind})"
        )

    def print_arg(self) -> Arith | Pred:
        """print の引数。算術式のほか、比較や真偽値も受け付ける。"""
        if self.peek().kind in ("TRUE", "FALSE", "NOT"):
            return self.predicate()
        left = self.arith()
        tok = self.accept("LT", "GT", "LE", "GE", "EQ")
        if tok is None:
            return left
        right = self.arith()
        match tok.kind:
            case "LT":
                return LT(left, right)
            case "GT":
                return GT(left, right)
            case "LE":
                return LE(left, right)
            case "GE":
                return GE(left, right)
            case "EQ":
                return EQ(left, right)
        raise AssertionError(tok.kind)

    def predicate(self) -> Pred:
        return self.pred_or()

    def pred_or(self) -> Pred:
        left = self.pred_and()
        while self.accept("OR"):
            left = Or(left, self.pred_and())
        return left

    def pred_and(self) -> Pred:
        left = self.pred_not()
        while self.accept("AND"):
            left = And(left, self.pred_not())
        return left

    def pred_not(self) -> Pred:
        if self.accept("NOT"):
            return Not(self.pred_not())
        return self.pred_atom()

    def pred_atom(self) -> Pred:
        if self.accept("TRUE"):
            return TrueConst()
        if self.accept("FALSE"):
            return FalseConst()
        if self.accept("LPAREN"):
            pred = self.predicate()
            self.expect("RPAREN")
            return pred
        left = self.arith()
        tok = self.accept("LT", "GT", "LE", "GE", "EQ")
        if tok is None:
            got = self.peek()
            raise ParseError(
                f"{got.line}:{got.col}: 比較演算子が必要です"
            )
        right = self.arith()
        match tok.kind:
            case "LT":
                return LT(left, right)
            case "GT":
                return GT(left, right)
            case "LE":
                return LE(left, right)
            case "GE":
                return GE(left, right)
            case "EQ":
                return EQ(left, right)
        raise AssertionError(tok.kind)

    def arith(self) -> Arith:
        left = self.term()
        while True:
            if self.accept("PLUS"):
                left = Add(left, self.term())
            elif self.accept("MINUS"):
                left = Sub(left, self.term())
            else:
                return left

    def term(self) -> Arith:
        left = self.atom()
        while True:
            if self.accept("TIMES"):
                left = Mul(left, self.atom())
            elif self.accept("DIVIDE"):
                left = Div(left, self.atom())
            else:
                return left

    def atom(self) -> Arith:
        tok = self.accept("NUMBER")
        if tok is not None:
            assert isinstance(tok.value, int)
            return Num(tok.value)
        tok = self.accept("IDENT")
        if tok is not None:
            assert isinstance(tok.value, str)
            return Var(tok.value)
        if self.accept("LPAREN"):
            expr = self.arith()
            self.expect("RPAREN")
            return expr
        got = self.peek()
        raise ParseError(
            f"{got.line}:{got.col}: 数値か変数が必要です ({got.kind})"
        )


def parse(tokens: list[Token]) -> Stmt:
    return Parser(tokens).parse()


def parse_predicate(tokens: list[Token]) -> Pred:
    return Parser(tokens).parse_predicate()
