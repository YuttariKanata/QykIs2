# QykIs2

QykIs2 github

```text
QykIs2/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # GUI & イベントループ (UIレイヤー)
│   └── engine/               # 数理・探索コア (エンジンレイヤー)
│       ├── curve.hpp         # 3次・4次・n次多項式の評価関数
│       ├── mod_sieve.hpp     # Mod p Sieve のテーブルと判定ロジック
│       ├── solver_engine.hpp # バックエンド非同期スレッド制御クラス
│       └── solver_engine.cpp
```
