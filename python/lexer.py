"""字句解析器。ソース文字列をトークン列に分割する。

このファイルは配布済み (課題で変更する必要はない)。
"""

from __future__ import annotations

from dataclasses import dataclass
import re


@dataclass(frozen=True)
class Token:
    kind: str
    value: str | int | None
    line: int
    col: int

    def __repr__(self) -> str:
        if self.value is None or self.value == self.kind.lower():
            return f"{self.kind}"
        return f"{self.kind}({self.value!r})"


class LexerError(Exception):
    pass


_KEYWORDS = {
    "begin": "BEGIN",
    "end": "END",
    "while": "WHILE",
    "do": "DO",
    "skip": "SKIP",
    "if": "IF",
    "then": "THEN",
    "else": "ELSE",
    "print": "PRINT",
    "true": "TRUE",
    "false": "FALSE",
    "not": "NOT",
    "and": "AND",
    "or": "OR",
}

# 長い演算子を先に書く。IDENT はキーワード判定を後段で行う。
_SPEC = [
    ("WS", r"[ \t\n\r]+"),
    ("COMMENT", r"#.*"),
    ("LE", r"<="),
    ("GE", r">="),
    ("EQ", r"=="),
    ("ASSIGN", r":="),
    ("LT", r"<"),
    ("GT", r">"),
    ("PLUS", r"\+"),
    ("MINUS", r"-"),
    ("TIMES", r"\*"),
    ("DIVIDE", r"/"),
    ("SEMICOLON", r";"),
    ("LPAREN", r"\("),
    ("RPAREN", r"\)"),
    ("NUMBER", r"[0-9]+"),
    ("IDENT", r"[A-Za-z_][A-Za-z0-9_]*"),
]

_PATTERN = re.compile(
    "|".join(f"(?P<{name}>{pat})" for name, pat in _SPEC)
)


def tokenize(src: str) -> list[Token]:
    tokens: list[Token] = []
    line = 1
    col = 1
    pos = 0
    n = len(src)

    while pos < n:
        m = _PATTERN.match(src, pos)
        if m is None:
            raise LexerError(f"{line}:{col}: 未知の文字 {src[pos]!r}")
        kind = m.lastgroup
        text = m.group()
        newlines = text.count("\n")
        if kind not in ("WS", "COMMENT"):
            if kind == "NUMBER":
                tokens.append(Token("NUMBER", int(text), line, col))
            elif kind == "IDENT":
                kw = _KEYWORDS.get(text)
                if kw is not None:
                    tokens.append(Token(kw, None, line, col))
                else:
                    tokens.append(Token("IDENT", text, line, col))
            else:
                tokens.append(Token(kind, None, line, col))
        if newlines:
            line += newlines
            col = len(text) - text.rfind("\n")
        else:
            col += len(text)
        pos = m.end()

    tokens.append(Token("EOF", None, line, col))
    return tokens
