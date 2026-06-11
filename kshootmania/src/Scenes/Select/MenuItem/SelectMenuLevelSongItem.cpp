#include "SelectMenuLevelSongItem.hpp"
#include "RuntimeConfig.hpp"

SelectMenuLevelSongItem::SelectMenuLevelSongItem(
	FilePathView chartFilePath,
	FilePathView songDirectoryPath,
	int32 difficultyIdx)
	: m_songDirectoryPath(songDirectoryPath)
	, m_difficultyIdx(difficultyIdx)
{
	auto chartInfo = std::make_shared<SelectChartInfo>(chartFilePath);
	if (!chartInfo->hasError())
	{
		m_chartInfo = chartInfo;
	}
	else
	{
		Logger << U"[ksm warning] SelectMenuLevelSongItem: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(chartInfo->errorString(), chartFilePath);
	}
}

SelectMenuLevelSongItem::SelectMenuLevelSongItem(
	std::shared_ptr<const SelectChartInfo> chartInfo,
	FilePathView songDirectoryPath,
	int32 difficultyIdx)
	: m_songDirectoryPath(songDirectoryPath)
	, m_difficultyIdx(difficultyIdx)
{
	if (chartInfo != nullptr && !chartInfo->hasError())
	{
		m_chartInfo = std::move(chartInfo);
	}
	else if (chartInfo != nullptr)
	{
		Logger << U"[ksm warning] SelectMenuLevelSongItem: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(chartInfo->errorString(), chartInfo->chartFilePath());
	}
}

void SelectMenuLevelSongItem::setSiblings(const std::array<SiblingDifficultyInfo, kNumDifficulties>& siblings)
{
	m_siblings = siblings;
}

FilePathView SelectMenuLevelSongItem::songDirectoryPath() const
{
	return m_songDirectoryPath;
}

int32 SelectMenuLevelSongItem::difficultyIdx() const
{
	return m_difficultyIdx;
}

bool SelectMenuLevelSongItem::chartExists() const
{
	return m_chartInfo != nullptr;
}

void SelectMenuLevelSongItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	if (m_chartInfo == nullptr)
	{
		return;
	}

	const FilePath chartFilePath = FilePath{ m_chartInfo->chartFilePath() };
	if (!FileSystem::Exists(chartFilePath))
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorChartFileNotFound));
		return;
	}

	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::No, none);
}

void SelectMenuLevelSongItem::decideAutoPlay(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	if (m_chartInfo == nullptr)
	{
		return;
	}

	const FilePath chartFilePath = FilePath{ m_chartInfo->chartFilePath() };
	if (!FileSystem::Exists(chartFilePath))
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorChartFileNotFound));
		return;
	}

	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::Yes, none);
}

FilePathView SelectMenuLevelSongItem::fullPath() const
{
	return m_songDirectoryPath;
}

const SelectChartInfo* SelectMenuLevelSongItem::chartInfoPtr(int difficultyIdx, [[maybe_unused]] FallbackForSingleChartYN fallbackForSingleChart) const
{
	if (difficultyIdx == m_difficultyIdx)
	{
		return m_chartInfo.get();
	}
	return nullptr;
}

Optional<HighScoreInfo> SelectMenuLevelSongItem::highScoreInfo(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx) const
{
	if (m_chartInfo == nullptr)
	{
		return none;
	}
	return context.fnGetHighScore(m_chartInfo->chartFilePath());
}

bool SelectMenuLevelSongItem::difficultyMenuExists() const
{
	return true;
}

Optional<int32> SelectMenuLevelSongItem::itemDifficultyIdx() const
{
	return m_difficultyIdx;
}

bool SelectMenuLevelSongItem::handleDifficultyChange(
	const SelectMenuEventContext& context,
	[[maybe_unused]] int32 currentDifficultyIdx,
	int32 delta) const
{
	// 現在の難易度を基準に、入力方向に存在する別難易度を探す
	if (delta > 0)
	{
		for (int32 i = m_difficultyIdx + 1; i < kNumDifficulties; ++i)
		{
			if (m_siblings[i].menuIndex >= 0)
			{
				context.fnJumpToItemWithDifficulty(m_siblings[i].menuIndex, i);
				return true;
			}
		}
	}
	else if (delta < 0)
	{
		for (int32 i = m_difficultyIdx - 1; i >= 0; --i)
		{
			if (m_siblings[i].menuIndex >= 0)
			{
				context.fnJumpToItemWithDifficulty(m_siblings[i].menuIndex, i);
				return true;
			}
		}
	}

	return false;
}

bool SelectMenuLevelSongItem::isFavoriteRegisterableItemType() const
{
	return true;
}

void SelectMenuLevelSongItem::setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", true },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
	});

	// 各難易度の存在有無とレベルを兄弟情報から設定
	for (int32 i = 0; i < kNumDifficulties; ++i)
	{
		const bool exists = m_siblings[i].menuIndex >= 0;
		const int32 levelIndex = exists ? m_siblings[i].level - 1 : -1;
		canvas.setSubCanvasParamValuesByTag(U"center", {
			{ U"difficulty{}Enabled"_fmt(i), exists },
			{ U"difficulty{}LevelIndex"_fmt(i), levelIndex },
			{ U"difficulty{}StyleState"_fmt(i), i == m_difficultyIdx ? U"" : U"levelSortOther" },
		});
	}

	if (m_chartInfo == nullptr)
	{
		throw Error{ U"SelectMenuLevelSongItem::setCanvasParamsCenter: m_chartInfo is null (songDirectoryPath={})"_fmt(m_songDirectoryPath) };
	}

	const HighScoreInfo highScore = context.fnGetHighScore(m_chartInfo->chartFilePath());
	const GaugeType gaugeType = RuntimeConfig::GetGaugeType();

	const FilePath jacketPath = m_chartInfo->jacketFilePath();
	const FilePath iconPath = m_chartInfo->iconFilePath();
	const SongInfoTextDisplayParams titleParams = MakeSongInfoTextDisplayParams(m_chartInfo->title(), m_chartInfo->titleTranslit());
	const SongInfoTextDisplayParams artistParams = MakeSongInfoTextDisplayParams(m_chartInfo->artist(), m_chartInfo->artistTranslit());

	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"title", titleParams.text },
		{ U"titleTranslit", titleParams.translitText },
		{ U"titleTranslitFadeRate", titleParams.translitFadeRate },
		{ U"artist", artistParams.text },
		{ U"artistTranslit", artistParams.translitText },
		{ U"artistTranslitFadeRate", artistParams.translitFadeRate },
		{ U"bpm", m_chartInfo->dispBPM() },
		{ U"jacketAuthor", m_chartInfo->jacketAuthor() },
		{ U"information", m_chartInfo->information() },
		{ U"chartAuthor", m_chartInfo->chartAuthor() },
		{ U"difficultyIndex", m_difficultyIdx },
		{ U"medalIndex", static_cast<int32>(highScore.medal()) },
		{ U"highScoreGradeIndex", static_cast<int32>(highScore.grade(gaugeType)) },
		{ U"highScore", highScore.score(gaugeType) },
		{ U"gaugePercentage", highScore.percent(gaugeType) },
		{ U"jacketFilePath", jacketPath },
		{ U"jacketActive", !jacketPath.isEmpty() && FileSystem::IsFile(jacketPath) },
		{ U"iconFilePath", iconPath },
		{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
	});
}

void SelectMenuLevelSongItem::setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	if (m_chartInfo == nullptr)
	{
		assert(false && "SelectMenuLevelSongItem::setCanvasParamsTopBottom: m_chartInfo is null");
		return;
	}

	// 常に自身の難易度のデータを表示
	const HighScoreInfo highScore = context.fnGetHighScore(m_chartInfo->chartFilePath());
	const GaugeType gaugeType = RuntimeConfig::GetGaugeType();

	const FilePath jacketPath = m_chartInfo->jacketFilePath();
	const FilePath iconPath = m_chartInfo->iconFilePath();
	const SongInfoTextDisplayParams titleParams = MakeSongInfoTextDisplayParams(m_chartInfo->title(), m_chartInfo->titleTranslit());
	const SongInfoTextDisplayParams artistParams = MakeSongInfoTextDisplayParams(m_chartInfo->artist(), m_chartInfo->artistTranslit());

	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", true },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", titleParams.text },
		{ U"titleTranslit", titleParams.translitText },
		{ U"titleTranslitFadeRate", titleParams.translitFadeRate },
		{ U"artist", artistParams.text },
		{ U"artistTranslit", artistParams.translitText },
		{ U"artistTranslitFadeRate", artistParams.translitFadeRate },
		{ U"levelIndex", m_chartInfo->level() - 1 },
		{ U"medalIndex", static_cast<int32>(highScore.medal()) },
		{ U"highScoreGradeIndex", static_cast<int32>(highScore.grade(gaugeType)) },
		{ U"gaugePercentage", highScore.percent(gaugeType) },
		{ U"jacketFilePath", jacketPath },
		{ U"jacketActive", !jacketPath.isEmpty() && FileSystem::IsFile(jacketPath) },
		{ U"iconFilePath", iconPath },
		{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
	});
}

void SelectMenuLevelSongItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	if (m_chartInfo == nullptr)
	{
		return;
	}
	System::ShowInFileManager(m_chartInfo->chartFilePath());
}

Optional<String> SelectMenuLevelSongItem::relativePathToCopy([[maybe_unused]] int32 difficultyIdx) const
{
	if (m_chartInfo == nullptr)
	{
		return none;
	}
	String relativePath = FsUtils::RelativePathFromSongsDir(m_chartInfo->chartFilePath());
	relativePath.replace(U'\\', U'/');
	return relativePath;
}
