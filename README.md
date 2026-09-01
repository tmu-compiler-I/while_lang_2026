# コンパイラ演習 2026 (情報システム実験 I)

## WhileLang から WebAssembly へのコンパイラ実装演習

WhileLang を WebAssembly テキスト形式 (`.wat`) へ翻訳するコンパイラを、**Python** か **C** で組み立てます。
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
cd python
python3 whilei.py ../test/assign.while     # 仮想スタック機械で実行
python3 whilei.py --trace ../test/assign.while
python3 whilec.py ../test/assign.while     # .wat を出力
```

C 実装の場合:

```bash
cd c
make
./whilei ../test/assign.while              # 仮想スタック機械で実行
./whilei --trace ../test/assign.while
./whilec ../test/assign.while              # .wat を出力
```

Windows のコマンドプロンプト / PowerShell では、Python は `py -3`、C は `c` ディレクトリで `build.bat` のあと `.\whilei ..\test\assign.while` です。Git Bash なら上の Unix コマンドのままで動きます。
