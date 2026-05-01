#pragma once
#include "HighScore/HighScoreInfo.hpp"
#include "HighScore/KscKey.hpp"

/// @brief 譜面・コースのハイスコア情報を遅延ロードするキャッシュ
class SelectHighScoreCache
{
private:
	HashTable<FilePath, HashTable<String, HighScoreInfo>> m_chartScoreMap;

	HashTable<FilePath, HashTable<String, HighScoreInfo>> m_courseScoreMap;

public:
	/// @brief キャッシュを破棄
	void clear();

	/// @brief 譜面のハイスコア情報を取得
	/// @param chartFilePath 譜面ファイルのパス
	/// @param key 取得するハイスコアの条件
	/// @return ハイスコア情報(該当エントリが無ければ空のHighScoreInfo)
	[[nodiscard]]
	HighScoreInfo getChartHighScore(FilePathView chartFilePath, const KscKey& key);

	/// @brief コースのハイスコア情報を取得
	/// @param courseFilePath コースファイル(.kco)のパス
	/// @param key 取得するハイスコアの条件
	/// @return ハイスコア情報(該当エントリが無ければ空のHighScoreInfo)
	[[nodiscard]]
	HighScoreInfo getCourseHighScore(FilePathView courseFilePath, const KscKey& key);
};
