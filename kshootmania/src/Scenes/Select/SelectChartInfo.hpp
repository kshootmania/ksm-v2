#pragma once
#include "kson/ChartData.hpp"
#include "HighScore/HighScoreInfo.hpp"
#include "Ini/FolderConfIni.hpp"

class SelectChartInfo
{
private:
	FilePath m_chartFilePath;

	kson::MetaChartData m_chartData;

	FolderConfIni m_folderConfIni;

	// 全条件のハイスコア情報(キー:gaugeType部分を除いたKscKey文字列)
	HashTable<String, HighScoreInfo> m_highScoreInfoMap;

	FilePath toFullPath(const std::string& u8Filename) const;

public:
	explicit SelectChartInfo(FilePathView chartFilePath);

	[[nodiscard]]
	String title() const;

	[[nodiscard]]
	String artist() const;

	[[nodiscard]]
	FilePath jacketFilePath() const;

	[[nodiscard]]
	String jacketAuthor() const;

	[[nodiscard]]
	FilePathView chartFilePath() const;

	[[nodiscard]]
	String chartAuthor() const;

	[[nodiscard]]
	int32 difficultyIdx() const;

	[[nodiscard]]
	int32 level() const;

	[[nodiscard]]
	String dispBPM() const;

	[[nodiscard]]
	double stdBPM() const;

	[[nodiscard]]
	double stdBPMForHispeedTypeChange() const;

	[[nodiscard]]
	FilePath previewBGMFilePath() const;

	[[nodiscard]]
	SecondsF previewBGMOffset() const;

	[[nodiscard]]
	Duration previewBGMDuration() const;

	[[nodiscard]]
	double previewBGMVolume() const;

	[[nodiscard]]
	FilePath iconFilePath() const;

	[[nodiscard]]
	String information() const;

	// 現在のプレイ設定に対応するハイスコア情報を取得
	[[nodiscard]]
	HighScoreInfo highScoreInfo() const;

	[[nodiscard]]
	bool hasError() const;

	[[nodiscard]]
	String errorString() const;
};
