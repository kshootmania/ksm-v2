#include "high_score_info.hpp"

namespace
{
	template <typename T>
	T GetFieldValue(const Array<String>& values, std::size_t index, T defaultValue)
	{
		if (values.size() <= index)
		{
			return defaultValue;
		}

		return static_cast<T>(Parse<int32>(values[index]));
	}
}

KscValue KscValue::applyPlayResult(const MusicGame::PlayResult& playResult) const
{
	return KscValue
	{
		.score = Max(score, playResult.score),
		.achievement = Max(achievement, playResult.achievement()),
		.grade = Max(grade, playResult.grade()),
		.percent = Max(percent, playResult.gaugePercentForHighScore()),
		.maxCombo = Max(maxCombo, playResult.maxCombo),
		.playCount = playCount + 1,
		.clearCount = clearCount + (playResult.achievement() >= Achievement::kCleared ? 1 : 0),
		.fullComboCount = fullComboCount + (playResult.achievement() >= Achievement::kFullCombo ? 1 : 0),
		.perfectCount = perfectCount + (playResult.achievement() >= Achievement::kPerfect ? 1 : 0),
	};
}

String KscValue::toString() const
{
	return U"{},{},{},{},{},{},{},{},{}"_fmt(
		score,
		static_cast<int32>(achievement),
		static_cast<int32>(grade),
		percent,
		maxCombo,
		playCount,
		clearCount,
		fullComboCount,
		perfectCount);
}

KscValue KscValue::FromString(const String& str)
{
	const Array<String> values = str.split(U',');

	const KscValue defaultValue{};

	return KscValue
	{
		.score = GetFieldValue<int32>(values, 0, defaultValue.score),
		.achievement = GetFieldValue<Achievement>(values, 1, defaultValue.achievement),
		.grade = GetFieldValue<Grade>(values, 2, defaultValue.grade),
		.percent = GetFieldValue<int32>(values, 3, defaultValue.percent),
		.maxCombo = GetFieldValue<int32>(values, 4, defaultValue.maxCombo),
		.playCount = GetFieldValue<int32>(values, 5, defaultValue.playCount),
		.clearCount = GetFieldValue<int32>(values, 6, defaultValue.clearCount),
		.fullComboCount = GetFieldValue<int32>(values, 7, defaultValue.fullComboCount),
		.perfectCount = GetFieldValue<int32>(values, 8, defaultValue.perfectCount),
	};
}
