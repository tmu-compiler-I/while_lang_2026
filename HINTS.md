# 課題のヒント

詰まったら上から。読んでください。

Python は `python/`、C は `c/` で作業します。命令名の対応は末尾の表です。

## まず確認すること

1. 配布されたコードそのままの状態で `../test/assign.while` が `--run` で `3` を出すこと
2. 触ってよいのは `virtual_stack` と `emit_wasm` だけ。パーサは動かさない
3. デバッグは次の順で行うと効率的

```bash
# Python (`python/` で)
python3 whilec.py --ast ../test/arith.while
python3 whilec.py --stack ../test/arith.while
python3 whilec.py --run ../test/arith.while
python3 test_day.py

# C (`c/` で)
./whilec --ast ../test/arith.while
./whilec --stack ../test/arith.while
./whilec --run ../test/arith.while
make test
```

  - `--ast` で木が期待どおりならパーサは正しいので、翻訳側 (自分の実装) を疑う
  - `--stack` で命令列を見て、README の翻訳例と同じ並びかを先に確認する
  - Wasm の前に `--run` が通ることをチェック

---

## 課題1 算術 (`-` `*` `/`)

`Add` がすでに同じ形です。コピーして演算の名前だけ変えます。

- 仮想スタック: 左辺を翻訳 → 右辺を翻訳 → 演算を 1 つ置く。
- Wasm: `Plus` が `i32.add` なので、引き算は `i32.sub`、かけ算は `i32.mul`。割り算の Wasm は配布済み (`i32.div_s`) です。

引き算 `10 - 3` は、スタックでは下が 10、上が 3、そのあと `-` です。先に右辺を積むと符号が逆になります。`print 3 - 10;` が `-7` になれば順は合っています。

C では命令は `I_MINUS` です。`I_SUB` という名前はありません。`ARITH_ADD` の `return bin_arith(a, instr_op(I_PLUS));` を真似ます。

`test/arith.while` の最後の `3 + 4 * 5` は 23 です。`*` を先に結合するのはパーサの仕事なので、翻訳は木の通りに左・右・演算と出せば足ります。

---

## 課題2 比較 (`>` `>=` `<=` `==`)

課題1と同じパターンです。`LT` がすでにあります。

真は 1、偽は 0 です。`print i > 1;` のように比較を `print` できます。

Wasm は符号付きです。

| 仮想命令 | Wasm |
|----------|------|
| `<` | `i32.lt_s` (配布済み) |
| `>` | `i32.gt_s` |
| `<=` | `i32.le_s` |
| `>=` | `i32.ge_s` |
| `==` | `i32.eq` |

C は `PRED_LT` → `instr_op(I_LT)` をコピーして `PRED_GT` と `I_GT` に変えます。

---

## 課題3 while

ここが本題です。`emit_wasm` の `block` / `loop` は書いてあるので、仮想命令の並びだけ決めます。

while は「条件を見る場所」と「抜ける場所」のラベルが 2 つ要ります。

```
while 条件 do 本体
```

を次の意味に分解します。

1. ここに戻って条件を見る (test)
2. 条件が偽なら抜ける (out)
3. 本体を実行する
4. test へ戻る
5. 抜けたあとはここ (out)

README の翻訳例と、`virtual_stack` の `While` にあるコメントの部品 (`LabelTest`, 条件, `GoFalse`, 本体, `GoTo`, `LabelOut`) を、この 1〜5 に割り当ててください。

よくある失敗:

- `GoFalse` と `GoTo` が逆: 一回も回らないか、無限ループ
- `LabelTest` と `LabelOut` に渡す `(test, out)` の組が違う: Wasm 生成が `block`/`loop` を閉じられない
- 本体が複数文なのに `begin` `end` が無い: パーサは `do` の直後の文ひとつだけを本体にする。入力側の問題です

### 確認

```bash
python3 whilec.py --run ../test/simple_loop.while
# 10
```

0 回のループも試すと安心です。`i := 5; while i < 0 do i := i + 1; print i;` は `5` のままです。

入れ子はラベルを毎回 `gen_label()` で新しく取れば足ります。`test/loop.while` が通れば組は合っています。

---

## 発展 if

条件を積んだあと、Wasm の `if` / `else` / `end` に対応する命令で then と else を
挟みます。仮想命令の名前は Python が `IfStart` `ElseOp` `IfEnd`、C が
`I_IF_START` `I_ELSE` `I_IF_END` です。emit 側は配布済みです。

`if x > 2 then print 1 else print 0;` で `x := 1` なら `0`、`x := 3` なら `1` です。

---

## 命令名 (Python / C)

| 役割 | Python | C |
|------|--------|---|
| 加算・減算・乗算・除算 | `Plus` `Minus` `Times` `Divide` | `I_PLUS` `I_MINUS` `I_TIMES` `I_DIVIDE` |
| 比較 | `Lt` `Gt` `Le` `Ge` `Eq` | `I_LT` `I_GT` `I_LE` `I_GE` `I_EQ` |
| while 入口 / 出口 | `LabelTest` `LabelOut` | `I_LABEL_TEST` `I_LABEL_OUT` |
| ジャンプ | `GoTo` `GoFalse` | `I_GOTO` `I_GOFALSE` |
| リストへ追加 | `+ [Minus()]` | `code_push` / `code_concat` / `bin_arith` |

- C で `malloc` した領域は解放しなくて構いません
