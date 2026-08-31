# コンパイラ演習 2026 (情報システム実験 I)

## WHILE言語からWebAssemblyへのコンパイラ実装演習

WHILE 言語を WebAssembly テキスト形式 (`.wat`) へ翻訳するコンパイラを、**Python** か **C** で組み立てます。
最初に一方を選び、途中で混ぜないでください。どちらを選んでも課題は同じです。

| ディレクトリ | 内容 |
|--------------|------|
| `python/` | Python 実装。詳細は [python/README.md](python/README.md) |
| `c/` | C 実装。詳細は [c/README.md](c/README.md) |
| `test/` | 入力プログラム (`.while`)。両言語で共有 |
| `HINTS.md` | 課題で詰まったときのヒント |

## ビルド方法

Python 実装の場合:

```bash
# Python
cd python
python3 whilec.py --run ../test/assign.while
```

C 実装の場合:

```
# C
cd c
make
./whilec --run ../test/assign.while
```
