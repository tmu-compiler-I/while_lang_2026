# Python 実装

WHILE を WebAssembly (`.wat`) へ翻訳します。字句解析と構文解析は配布済みです。
今日書くのは、構文木 → 仮想スタック命令 → Wasm です。

入力プログラムは `../test/*.while` です。C 版は [`../c/`](../c/) にあります。

## 準備

Python 3.10 以降。追加ライブラリは不要です。Windows では WSL を推奨します。

```bash
python3 --version    # 3.10 以上 (match 文)
python3 test_day.py  # 構文解析と assign は通る
```

## 最初に動かす

```bash
python3 whilei.py ../test/assign.while
# 3

python3 whilei.py --trace ../test/assign.while   # 命令・スタック・変数を1ステップずつ
python3 whilec.py --stack ../test/assign.while
python3 whilec.py ../test/assign.while
# wrote ../test/assign.wat
```

`.wat` は https://3tty0n.github.io/wonline にドロップするとブラウザで実行できます。

`whilei.py` は Wasm を使わず仮想スタック命令を実行します。デバッグは `--trace` でスタックと変数の変化を追えます。

```bash
python3 test_day.py
```

`TODO` と出る行が、まだ書いていない課題です。詰まったら [HINTS.md](../HINTS.md) を見てください。

## WHILE 言語

```
S ::= x := a
    | skip
    | S ; S
    | begin S end
    | while P do S
    | print a
    | if P then S else S          (発展課題)

a ::= x | n | (a) | a + a | a - a | a * a | a / a

P ::= true | false | not P | P and P | P or P
    | a < a | a > a | a <= a | a >= a | a == a
```

コメントは `#` から行末までです。整数は 32 ビット符号付き、変数は宣言せずに使えます (初期値 0)。

```
# ../test/assign.while
i := 1;
j := 2;
print i + j;
```

## コンパイラの流れ

```
.while ソース
    │  lexer.py     字句解析 (配布)
    ▼
トークン列
    │  parser.py    構文解析 (配布)
    ▼
構文木 syntax.py
    │  virtual_stack.py   ← 課題 1〜3
    ▼
仮想スタック命令
    │  emit_wasm.py       ← 課題 1, 2
    ▼
.wat (WebAssembly テキスト)
```

字句解析器と構文解析器は読めるように短く書いてあります。変更する必要はありません。

## 仮想スタック機械

命令はオペランドスタックを操作します。`+` は上から 2 つを取り、和を積む、という具合です。

| 命令 | 動作 |
|------|------|
| `push n` | 定数 `n` を積む |
| `rvalue x` | 変数 `x` の値を積む |
| `lpush x` | 先頭を取り、変数 `x` に入れる |
| `+` `-` `*` `/` | 二項演算。下の値 `a`、上の値 `b` に対し `a op b` |
| `<` `>` `<=` `>=` `==` | 比較。真なら 1、偽なら 0 |
| `label L` | ジャンプ先。それ以外の効果はない |
| `goto L` | `L` へ飛ぶ |
| `gofalse L` | 先頭を取り、0 なら `L` へ飛ぶ |
| `print` | 先頭を取り、出力する |

### 翻訳例: 代入

入力 `a := a * 2;`

```
rvalue  a
push    2
*
lpush   a
```

左辺の `a` は格納先なので `lpush`、右辺の `a` は値なので `rvalue` です。

### 翻訳例: while

入力 `while i < 10 do i := i + 1;`

```
label   L.0          # 条件判定 (test)
rvalue  i
push    10
<
gofalse L.1          # 偽なら脱出 (out)
rvalue  i
push    1
+
lpush   i
goto    L.0          # 条件判定へ戻る
label   L.1
```

実装では `LabelTest(test, out)` が入口ラベル、`LabelOut(test, out)` が出口ラベルです。
この 2 つは後段の Wasm 生成が `block` / `loop` に対応づけるために、ラベル名の組を持っています。

## WebAssembly

Wasm もスタック機械です。仮想命令とほぼ一対一で落ちます。

| 仮想命令 | Wasm |
|----------|------|
| `push n` | `i32.const n` |
| `rvalue x` | `global.get $x` |
| `lpush x` | `global.set $x` |
| `+` `-` `*` `/` | `i32.add` / `sub` / `mul` / `div_s` |
| `==` `<` `>` `<=` `>=` | `i32.eq` / `lt_s` / `gt_s` / `le_s` / `ge_s` |
| `gofalse L` | `i32.eqz` のあと `br_if $L` |
| `goto L` | `br $L` |
| `print` | `call $print` |

while は次の形になります (生成コードは配布済みです)。

```
(block $L.1           ;; 脱出先
  (loop $L.0          ;; 繰り返し先
    ...条件...
    i32.eqz
    br_if $L.1        ;; 偽なら block を抜ける
    ...本体...
    br $L.0           ;; loop の先頭へ
  )
)
```

生成されるモジュールは `print` をホストから import し、変数を mutable global、本体を `main` として export します。

## 課題

触るファイルは `virtual_stack.py` と `emit_wasm.py` だけです。
`Add` と `LT` と `Plus` / `Lt` / `i32.add` / `i32.lt_s` が実装済みなので、それをコピーして書き換えてください。

### 課題1 算術演算

`Sub` / `Mul` / `Div` を仮想スタック命令へ翻訳し、対応する Wasm (`i32.sub`, `i32.mul`, `i32.div_s`) を出力する。

```bash
python3 whilei.py ../test/arith.while
# 7
# 8
# 4
# 23
```

`3 + 4 * 5` が 23 になるのは、構文解析器が `*` を `+` より先に結合するためです。

### 課題2 比較演算

`GT` / `GE` / `LE` / `EQ` を翻訳し、`i32.gt_s` / `ge_s` / `le_s` / `eq` を出力する。

```bash
python3 whilei.py ../test/cmp.while
# 1 1 0 1 1 0
```

真は 1、偽は 0 です。

### 課題3 while

`While` をラベルとジャンプへ翻訳する。`emit_wasm.py` 側の `block` / `loop` は書いてあります。

`virtual_stack.py` の `While` 節に、部品を並べるためのコメントがあります。上の翻訳例と同じ順になるように組み立ててください。

```bash
python3 whilei.py ../test/simple_loop.while
# 10

python3 whilei.py ../test/fact.while
# 120
```

`../test/loop.while` は二重ループです。通れば入れ子のラベルも合っています。

### 発展課題 if

時間が余ったら `If` を実装してください。条件のあとに `IfStart()`、then 節、`ElseOp()`、else 節、`IfEnd()` です。Wasm の `if` / `else` / `end` は配布済みです。

```bash
python3 whilei.py ../test/ifstmt.while
# 1
```

## ファイル

| ファイル | 役割 |
|----------|------|
| `syntax.py` | 構文木の型 |
| `lexer.py` | 字句解析 (配布) |
| `parser.py` | 再帰下降パーサ (配布) |
| `virtual_stack.py` | 構文木 → 仮想スタック命令 (**課題**) |
| `emit_wasm.py` | 仮想スタック命令 → WAT (**課題**) |
| `interpret.py` | 仮想スタック命令の実行 (配布) |
| `whilec.py` | コンパイラ (`.wat` / `--ast` / `--stack`) |
| `whilei.py` | 仮想スタック機械で実行 (`--trace` で可視化) |
| `test_day.py` | 進捗確認 |

## コマンド

```bash
python3 whilec.py --ast ../test/assign.while
python3 whilec.py --stack ../test/assign.while
python3 whilec.py -o out.wat ../test/assign.while
python3 whilei.py ../test/assign.while
python3 whilei.py --trace ../test/assign.while
```
