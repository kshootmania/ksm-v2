#include "select_chart_info.hpp"
#include "high_score/ksc_io.hpp"
#include "kson/io/ksh_io.hpp"

namespace
{
	HighScoreInfo LoadHighScoreInfo(FilePathView chartFilePath)
	{
		// TODO(alphaまで): 実際の設定を反映
		const KscKey condition
		{
			.gaugeType = GaugeType::kNormalGauge,
			.turnMode = TurnMode::kNormal,
			.btPlayMode = JudgmentPlayMode::kOn,
			.fxPlayMode = JudgmentPlayMode::kOn,
			.laserPlayMode = JudgmentPlayMode::kOn,
		};

		return KscIo::ReadHighScoreInfo(chartFilePath, condition);
	}
}

FilePath SelectChartInfo::toFullPath(const std::string& u8Filename) const
{
	return FileSystem::PathAppend(FileSystem::ParentPath(m_chartFilePath), Unicode::FromUTF8(u8Filename));
}

SelectChartInfo::SelectChartInfo(FilePathView chartFilePath)
	: m_chartFilePath(chartFilePath)
	, m_chartData(kson::LoadKSHMetaChartData(chartFilePath.narrow()))
	, m_highScoreInfo(LoadHighScoreInfo(chartFilePath))
{
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

FilePath SelectChartInfo::previewBGMFilePath() const
{
	return toFullPath(m_chartData.audio.bgm.filename);
}

double SelectChartInfo::previewBGMOffsetSec() const
{
	return m_chartData.audio.bgm.preview.offset / 1000.0;
}

double SelectChartInfo::previewBGMDurationSec() const
{
	return m_chartData.audio.bgm.preview.duration / 1000.0;
}

double SelectChartInfo::previewBGMVolume() const
{
	return m_chartData.audio.bgm.vol;
}

FilePath SelectChartInfo::iconFilePath() const
{
	return toFullPath(m_chartData.meta.iconFilename);
}

String SelectChartInfo::information() const
{
	return Unicode::FromUTF8(m_chartData.meta.information);
}

const HighScoreInfo& SelectChartInfo::highScoreInfo() const
{
	return m_highScoreInfo;
}

bool SelectChartInfo::hasError() const
{
	return m_chartData.error != kson::Error::None;
}

String SelectChartInfo::errorString() const
{
	return Unicode::FromUTF8(kson::GetErrorString(m_chartData.error));
}
