#include "ksc_file.hpp"

HighScoreInfo ReadHighScoreInfo(FilePathView kscFilePath, const HighScoreCondition& condition)
{
	TextReader reader(kscFilePath);
	if (!reader)
	{
		return HighScoreInfo{};
	}

	const String search = condition.toKscKey() + U"=";

	String line;
	while (reader.readLine(line))
	{
		if (line.starts_with(search))
		{
			const String value = line.substr(search.length());
			return HighScoreInfo::FromKscValue(value);
		}
	}

	return HighScoreInfo{};
}
