#include "SelectChartInfo.hpp"
#include "HighScore/KscIO.hpp"
#include "HighScore/KscKey.hpp"
#include "Ini/ConfigIni.hpp"
#include "kson/IO/KshIO.hpp"
#include "RuntimeConfig.hpp"

namespace
{
	KscKey CreateKscKeyFromConfig()
	{
		return KscKey
		{
			.gaugeType = RuntimeConfig::GetGaugeType(),
			.turnMode = RuntimeConfig::GetTurnMode(),
			.playbackSpeed = RuntimeConfig::GetPlaybackSpeed(),
			.btPlayMode = RuntimeConfig::GetJudgmentPlayModeBT(),
			.fxPlayMode = RuntimeConfig::GetJudgmentPlayModeFX(),
			.laserPlayMode = RuntimeConfig::GetJudgmentPlayModeLaser(),
		};
	}
}

FilePath SelectChartInfo::toFullPath(const std::string& u8Filename) const
{
	return FileSystem::PathAppend(FileSystem::ParentPath(m_chartFilePath), Unicode::FromUTF8(u8Filename));
}

SelectChartInfo::SelectChartInfo(FilePathView chartFilePath)
	: m_chartFilePath(chartFilePath)
	, m_chartData(FsUtils::HasKsonExtension(chartFilePath)
		? kson::LoadKsonMetaChartData(chartFilePath.toUTF8())
		: kson::LoadKshMetaChartData(chartFilePath.toUTF8()))
	, m_folderConfIni(FolderConfIni::Load(chartFilePath))
{
	KscIO::ReadAllHighScoreInfo(chartFilePath, &m_highScoreInfoMap);
}

String SelectChartInfo::title() const
{
	return Unicode::FromUTF8(m_chartData.meta.title);
}

String SelectChartInfo::artist() const
{
	return Unicode::FromUTF8(m_chartData.meta.artist);
}

FilePath SelectChartInfo::jacketFilePath() const
{
	return toFullPath(m_chartData.meta.jacketFilename);
}

String SelectChartInfo::jacketAuthor() const
{
	return Unicode::FromUTF8(m_chartData.meta.jacketAuthor);
}

FilePathView SelectChartInfo::chartFilePath() const
{
	return m_chartFilePath;
}

String SelectChartInfo::chartAuthor() const
{
	return Unicode::FromUTF8(m_chartData.meta.chartAuthor);
}

int32 SelectChartInfo::difficultyIdx() const
{
	return m_chartData.meta.difficulty.idx;
}

int32 SelectChartInfo::level() const
{
	return m_chartData.meta.level;
}

String SelectChartInfo::dispBPM() const
{
	return Unicode::FromUTF8(m_chartData.meta.dispBPM);
}

double SelectChartInfo::stdBPM() const
{
	return m_chartData.meta.stdBPM;
}

double SelectChartInfo::stdBPMForHispeedTypeChange() const
{
	// TODO: kson形式はksh形式とは異なりフルで読み込む前提のため、bpmの先頭要素を採用できる見込み

	const double std = stdBPM();
	if (std > 0.0)
	{
		return std;
	}

	const String dispBPMStr = dispBPM();
	if (dispBPMStr.isEmpty())
	{
		return kDefaultBPM;
	}

	const Array<String> parts = dispBPMStr.split(U'-');
	if (parts.empty())
	{
		return kDefaultBPM;
	}

	return ParseOr<double>(parts.back(), kDefaultBPM);
}

FilePath SelectChartInfo::previewBGMFilePath() const
{
	return toFullPath(m_chartData.audio.bgm.filename);
}

SecondsF SelectChartInfo::previewBGMOffset() const
{
	return SecondsF{ m_chartData.audio.bgm.preview.offset / 1000.0 };
}

Duration SelectChartInfo::previewBGMDuration() const
{
	return Duration{ m_chartData.audio.bgm.preview.duration / 1000.0 };
}

double SelectChartInfo::previewBGMVolume() const
{
	return m_chartData.audio.bgm.vol * m_folderConfIni.volumeScale;
}

FilePath SelectChartInfo::iconFilePath() const
{
	return toFullPath(m_chartData.meta.iconFilename);
}

String SelectChartInfo::information() const
{
	return Unicode::FromUTF8(m_chartData.meta.information);
}

HighScoreInfo SelectChartInfo::highScoreInfo() const
{
	const String key = CreateKscKeyFromConfig().toStringWithoutGaugeType();
	if (auto it = m_highScoreInfoMap.find(key); it != m_highScoreInfoMap.end())
	{
		return it->second;
	}
	return HighScoreInfo{};
}

bool SelectChartInfo::hasError() const
{
	return m_chartData.error != kson::ErrorType::None;
}

String SelectChartInfo::errorString() const
{
	return Unicode::FromUTF8(kson::GetErrorString(m_chartData.error));
}
