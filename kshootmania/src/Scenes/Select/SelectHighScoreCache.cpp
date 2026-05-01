#include "SelectHighScoreCache.hpp"
#include "HighScore/KscIO.hpp"

namespace
{
	const HashTable<String, HighScoreInfo>& LoadOrGetChartMap(
		HashTable<FilePath, HashTable<String, HighScoreInfo>>* pCache,
		FilePathView chartFilePath)
	{
		const FilePath key{ chartFilePath };
		if (auto it = pCache->find(key); it != pCache->end())
		{
			return it->second;
		}

		HashTable<String, HighScoreInfo> map;
		KscIO::ReadAllHighScoreInfo(chartFilePath, &map);
		const auto it = pCache->emplace(key, std::move(map)).first;
		return it->second;
	}

	const HashTable<String, HighScoreInfo>& LoadOrGetCourseMap(
		HashTable<FilePath, HashTable<String, HighScoreInfo>>* pCache,
		FilePathView courseFilePath)
	{
		const FilePath key{ courseFilePath };
		if (auto it = pCache->find(key); it != pCache->end())
		{
			return it->second;
		}

		HashTable<String, HighScoreInfo> map;
		KscIO::ReadAllCourseHighScoreInfo(courseFilePath, &map);
		const auto it = pCache->emplace(key, std::move(map)).first;
		return it->second;
	}
}

void SelectHighScoreCache::clear()
{
	m_chartScoreMap.clear();
	m_courseScoreMap.clear();
}

HighScoreInfo SelectHighScoreCache::getChartHighScore(FilePathView chartFilePath, const KscKey& key)
{
	const auto& map = LoadOrGetChartMap(&m_chartScoreMap, chartFilePath);
	const String mapKey = key.toStringWithoutGaugeType();
	if (auto it = map.find(mapKey); it != map.end())
	{
		return it->second;
	}
	return HighScoreInfo{};
}

HighScoreInfo SelectHighScoreCache::getCourseHighScore(FilePathView courseFilePath, const KscKey& key)
{
	const auto& map = LoadOrGetCourseMap(&m_courseScoreMap, courseFilePath);
	const String mapKey = key.toStringWithoutGaugeType();
	if (auto it = map.find(mapKey); it != map.end())
	{
		return it->second;
	}
	return HighScoreInfo{};
}
