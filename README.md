# QykIs2

QykIs2 github

```text
QykIs2/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # UI & イベントループ (ImGui)
│   └── engine/
│       ├── mod_sieve.hpp     # 1次フィルター (ACL modint + Horner法)
│       ├── exact_eval.hpp    # 2次フィルター (isqrt128 + 完全平方判定)
│       ├── solver_engine.hpp # [新規] 非同期スレッド制御 & 有理点構造体
│       └── solver_engine.cpp # [新規] パイプライン探索ルーチン
```

解の見つけ方

```text
[入力: 曲線定義, Height上限 (max_d, max_X)]
   │
   └─ 1. d = 1 ... max_d のループ (分母スケール)
         │
         └─ 2. X = -max_X ... max_X のループ (分子候補)
               │
               ├─ [チェックA] gcd(X, d) == 1 か？ (約数カット)
               │
               ├─ [チェックB] Mod Sieve フィルター (余り計算のみで98%カット)
               │
               └─ [チェックC] T(X, d) の評価と isqrt による整数平方数判定
                     │
                     └─ 解発見！ (X/d^2, Y/d^3) を有理数型(Rational)に変換して記録
```
