#pragma once
#include "music_game/play_result.hpp"

struct KscValue
{
	int32 score = 0;

	Achievement achievement = Achievement::kNone;

	Grade grade = Grade::kNoGrade;

	int32 percent = 0;

	int32 maxCombo = 0;

	int32 playCount = 0;

	int32 clearCount = 0;

	int32 fullComboCount = 0;

	int32 perfectCount = 0;

	/// @brief プレイ結果を反映後のKscValueを返す
	/// @param playResult プレイ結果
	/// @return KscValue
	KscValue applyPlayResult(const MusicGame::PlayResult& playResult) const;

	/// @brief 文字列に変換
	/// @return 変換後の文字列
	String toString() const;

	static KscValue FromString(const String& kscValue);
};
