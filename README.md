K-Shoot MANIA v1 Remastered
========================

## 開発状況

https://github.com/m4saka/kshootmania-v1-cpp/wiki#%E9%96%8B%E7%99%BA%E7%8A%B6%E6%B3%81

## 開発のコンセプト

- KSM v1(HSP版)をC++で完全再現することを目標
    - 次期バージョンへの足がかりとなるコードベース作成
- 組み直しによる各種改善
    - パフォーマンス改善
    - LASER判定の改善
    - 譜面データのハッシュ値をキーとしたスコア管理・コース作成による利便性向上
- kson形式の実験的導入
    - kson形式を実行中の内部データの構造として使用する実験
    - 追加機能の部分的サポート

## ビルドに必要な環境 / Build Requirements

- Visual Studio 2022
    - Visual Studio Installer上で「C++ によるデスクトップ開発」にチェックを付けてインストールしてください /  
      Install "Desktop development with C++" from the Visual Studio Installer
- OpenSiv3D 0.6.6 (installed to Visual Studio 2022)
    - 下記からインストーラをダウンロードして実行してください / Download and run the installer below
        - https://github.com/Siv3D/OpenSiv3D#downloads
