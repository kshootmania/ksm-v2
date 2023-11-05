#include "high_score_info.hpp"

namespace
{
	template <typename T>
	T GetKscValue(const Array<String>& values, std::size_t index, T defaultValue)
	{
		if (values.size() <= index)
		{
			return defaultValue;
		}

		return static_cast<T>(Parse<int32>(values[index]));
	}
}

HighScoreInfo HighScoreInfo::fromKscValue(const String& kscValue)
{
	HighScoreInfo highScoreInfo;

	const Array<String> values = kscValue.split(U',');
	highScoreInfo.score = GetKscValue<int32>(values, 0, highScoreInfo.score);
	highScoreInfo.medal = GetKscValue<Medal>(values, 1, highScoreInfo.medal);
	highScoreInfo.grade = GetKscValue<Grade>(values, 2, highScoreInfo.grade);
	highScoreInfo.percent = GetKscValue<int32>(values, 3, highScoreInfo.percent);
	highScoreInfo.maxCombo = GetKscValue<int32>(values, 4, highScoreInfo.maxCombo);
	highScoreInfo.playCount = GetKscValue<int32>(values, 5, highScoreInfo.playCount);
	highScoreInfo.clearCount = GetKscValue<int32>(values, 6, highScoreInfo.clearCount);
	highScoreInfo.fullComboCount = GetKscValue<int32>(values, 7, highScoreInfo.fullComboCount);

	return highScoreInfo;
}
