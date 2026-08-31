# C 実装

WHILE 言語コンパイラの C 言語実装です。
WHILE を WebAssembly (`.wat`) へコンパイルします。

課題の内容・言語仕様は [python/README.md](../python/README.md) と同じです。入力
プログラムは `../test/*.while` を共有します。

## コンパイル方法

```bash
make
./whilei ../test/assign.while    # 仮想スタック機械で実行
./whilec ../test/assign.while    # .wat を出力
make test
```

触るファイルは `virtual_stack.c` と `emit_wasm.c` だけです。詰まったら [HINTS.md](../HINTS.md) を見てください。

## 注意

Python の `match` はここでは `switch`、`Minus()` は `I_MINUS` です。malloc した
結果は解放しなくて構いません。
