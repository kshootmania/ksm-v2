#pragma once
#include "MusicGame/PlayOption.hpp"

struct TestPlayArgs
{
	FilePath chartFilePath;
	MusicGame::IsAutoPlayYN isAutoPlay = MusicGame::IsAutoPlayYN::No;
	MusicGame::TestPlayOption testPlayOption;
};

/// @brief コマンドライン引数を解析してテストプレイ引数を返す
/// @param pShouldExit エラーが発生してアプリケーションを終了すべき場合にtrueが設定される
[[nodiscard]]
Optional<TestPlayArgs> ParseTestPlayArgs(bool* pShouldExit);
