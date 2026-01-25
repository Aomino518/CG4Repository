# CONTRIBUTING.md

## ガイドライン

このリポジトリに対するコントリビューションは以下の規約に従ってください。

### メモリ管理
- 生の `new` / `delete` の使用は禁止します。代わりに標準ライブラリのスマートポインタを使用してください。
  - 所有権の単一化には `std::unique_ptr<T>` を使用します。
  - 共有所有権が本当に必要な場合のみ `std::shared_ptr<T>` を検討します。
- ファクトリ関数や生成関数は所有権を `std::unique_ptr<T>` で返却することで明示的に移譲してください。

### シングルトン
- シングルトンは `new` を使った動的確保を避け、Meyers シングルトン（関数内静的インスタンス）を使用してください。
  - 例: `static SceneManager& GetInstance() { static SceneManager instance; return instance; }`
- グローバル破壊順序問題およびリークを避けるため、静的オブジェクトを推奨します。

