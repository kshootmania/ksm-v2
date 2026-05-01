#pragma once
#include "kson/ChartData.hpp"
#include "Ini/FolderConfIni.hpp"

class SelectChartInfo
{
private:
	FilePath m_chartFilePath;

	kson::MetaChartData m_chartData;

	FolderConfIni m_folderConfIni;

	// 検索用文字列を連結したもの
	String m_joinedTextForSearch;

	FilePath toFullPath(const std::string& u8Filename) const;

public:
	explicit SelectChartInfo(FilePathView chartFilePath);

	SelectChartInfo(FilePathView chartFilePath, const kson::MetaChartData& chartData, const FolderConfIni& folderConfIni);

	[[nodiscard]]
	String title() const;

	[[nodiscard]]
	String titleTranslit() const;

	[[nodiscard]]
	String artist() const;

	[[nodiscard]]
	String artistTranslit() const;

	[[nodiscard]]
	FilePath titleImgFilePath() const;

	[[nodiscard]]
	FilePath artistImgFilePath() const;

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

	[[nodiscard]]
	bool hasError() const;

	[[nodiscard]]
	String errorString() const;

	[[nodiscard]]
	StringView joinedTextForSearch() const;
};
