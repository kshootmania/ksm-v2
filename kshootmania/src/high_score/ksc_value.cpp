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
