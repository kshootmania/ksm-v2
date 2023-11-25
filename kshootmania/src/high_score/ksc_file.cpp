#include "ksc_file.hpp"

HighScoreInfo ReadHighScoreInfo(FilePathView kscFilePath, const KscKey& condition)
{
	HighScoreInfo highScoreInfo{};

	TextReader reader(kscFilePath);
	if (!reader)
	{
		return highScoreInfo;
	}

	const String searchEasy = condition.withGaugeType(GaugeType::kEasyGauge).toString() + U"=";
	const String searchNormal = condition.withGaugeType(GaugeType::kNormalGauge).toString() + U"=";
	const String searchHard = condition.withGaugeType(GaugeType::kHardGauge).toString() + U"=";

	String line;
	while (reader.readLine(line))
	{
		if (line.starts_with(searchEasy))
		{
			const String valueStr = line.substr(searchEasy.length());
			highScoreInfo.easyGauge = KscValue::FromString(valueStr);
		}

		if (line.starts_with(searchNormal))
		{
			const String valueStr = line.substr(searchNormal.length());
			highScoreInfo.normalGauge = KscValue::FromString(valueStr);
		}

		if (line.starts_with(searchHard))
		{
			const String valueStr = line.substr(searchHard.length());
			highScoreInfo.hardGauge = KscValue::FromString(valueStr);
		}
	}

	return highScoreInfo;
}
