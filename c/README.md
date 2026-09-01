# C 実装

WhileLang コンパイラの C 言語実装です。
WhileLang を WebAssembly (`.wat`) へコンパイルします。

課題の内容・言語仕様は [python/README.md](../python/README.md) と同じです。入力
プログラムは `../test/*.while` を共有します。

## コンパイル方法

```bash
make
./whilei ../test/assign.while    # 仮想スタック機械で実行
./whilei --trace ../test/assign.while
./whilec ../test/assign.while    # .wat を出力
make test
```

Windows では Git Bash / MSYS2 なら上と同じです。コマンドプロンプトなら `build.bat` のあと `whilei ..\test\assign.while` です。`gcc` (MinGW / w64devkit) か Visual Studio の `cl` が必要です。

触るファイルは `virtual_stack.c` と `emit_wasm.c` だけです。詰まったら [HINTS.md](../HINTS.md) を見てください。

## 注意

Python の `match` はここでは `switch`、`Minus()` は `I_MINUS` です。malloc した
結果は解放しなくて構いません。
