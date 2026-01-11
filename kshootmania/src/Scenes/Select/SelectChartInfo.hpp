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

	String title() const;

	String artist() const;

	FilePath jacketFilePath() const;

	String jacketAuthor() const;

	FilePathView chartFilePath() const;

	String chartAuthor() const;

	int32 difficultyIdx() const;

	int32 level() const;

	String dispBPM() const;

	double stdBPM() const;

	double stdBPMForHispeedTypeChange() const;

	FilePath previewBGMFilePath() const;

	SecondsF previewBGMOffset() const;

	Duration previewBGMDuration() const;

	double previewBGMVolume() const;

	FilePath iconFilePath() const;

	String information() const;

	// 現在のプレイ設定に対応するハイスコア情報を取得
	HighScoreInfo highScoreInfo() const;

	bool hasError() const;

	String errorString() const;
};
