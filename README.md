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

main.cppが肥大化したのでこうするつもり...?

```text
src/
├── main.cpp                  # 初期化とメインループのみ（50行程度）
├── app_state.hpp             # アプリ全体の状態 (ViewMode, AppState 構造体)
├── math/
│   ├── integer_math.hpp    # clz128, isqrt128, check_perfect_square, SQ_MOD256
│   ├── number_theory.hpp   # generate_primes
│   ├── curve_eval.hpp      # eval_exact_deg3/4/5 (多項式評価)
│   └── curve_transform.hpp # 有理変換・標準化・逆変換 (先ほど設計したもの)
├── utils/
│   ├── string_utils.hpp      # to_string_128
│   └── history_manager.hpp   # HistoryManager / SearchSession
├── ui/
|   ├── ui_components.hpp     # Splitter ハンドル
|   ├── view_workspace.cpp    # メインの探索ペイン
|   ├── view_history.cpp      # 履歴画面
|   └── view_settings.cpp     # 設定画面
└── engine/
    ├── mod_sieve.hpp       # エンジン用フィルタリング (math/number_theoryを#include)
    └── engine.hpp          # 探索のメインループ
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

入力の変換

```text
[UI: Input Strings] 
       │ (parse_rational_input)
       ▼
[一般形の有理数係数 (mpq_t)]
       │
       ├─► [HistoryManager に保存 (SearchSession)]
       │
       ▼
[curve_transform::normalize()]
       │
       ├─► 標準形の係数 (CurveConfig / __int128_t) ──► [Engine::start_search()]
       │                                                         │ (解 u, v 発見)
       └─► 変換情報 (CurveTransformInfo) ──────────┐              ▼
                                                  └──► [逆変換: (u,v) -> (x,y)]
                                                                  │
                                                                  ▼
                                                          [PointLog に追加]
```
