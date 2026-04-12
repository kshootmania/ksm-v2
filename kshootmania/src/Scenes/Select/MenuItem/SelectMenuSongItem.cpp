#include "SelectMenuSongItem.hpp"
#include "Graphics/FontUtils.hpp"
#include "Scenes/Select/SelectDifficultyMenu.hpp"
#include "RuntimeConfig.hpp"

SelectMenuSongItem::SelectMenuSongItem(FilePathView fullPath)
	: m_fullPath(fullPath)
{
	Array<FilePath> chartFilePaths;

	if (FileSystem::IsFile(fullPath))
	{
		// 個別の譜面ファイルの場合
		if (FsUtils::HasChartExtension(fullPath))
		{
			chartFilePaths.emplace_back(fullPath);
			m_isSingleChartItem = true;
		}
	}
	else if (FileSystem::IsDirectory(fullPath))
	{
		// ディレクトリの場合
		chartFilePaths = FsUtils::GetChartFilePathsPreferringKson(fullPath);
	}
	else
	{
		Logger << U"[ksm warning] SelectMenuSongItem::SelectMenuSongItem: Path does not exist (path:'{}')"_fmt(fullPath);
	}

	for (const auto& chartFilePath : chartFilePaths)
	{
		if (!FsUtils::HasChartExtension(chartFilePath))
		{
			continue;
		}

		auto chartInfo = std::make_unique<SelectChartInfo>(chartFilePath);

		if (chartInfo->hasError())
		{
			Logger << U"[ksm warning] SelectMenuSongItem::SelectMenuSongItem: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(chartInfo->errorString(), chartFilePath);
			continue;
		}

		int32 difficultyIdx = chartInfo->difficultyIdx();
		if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx) [[unlikely]]
		{
			// 未知の難易度の場合は一番右の難易度にする
			Logger << U"[ksm warning] SelectMenuSongItem::SelectMenuSongItem: Difficulty index out of range (difficultyIdx:{}, chartFilePath:'{}')"_fmt(difficultyIdx, chartFilePath);
			difficultyIdx = kNumDifficulties - 1;
		}

		if (m_chartInfos[difficultyIdx] != nullptr) [[unlikely]]
		{
			Logger << U"[ksm warning] SelectMenuSongItem::SelectMenuSongItem: Skip duplication (difficultyIdx:{}, chartFilePath:'{}')"_fmt(difficultyIdx, chartFilePath);
			continue;
		}

		m_chartInfos[difficultyIdx] = std::move(chartInfo);

		m_chartExists = true;
	}
}

void SelectMenuSongItem::decide(const SelectMenuEventContext& context, int32 difficultyIdx)
{
	// 単一譜面項目の場合はdifficultyIdxに関係なくchartInfoPtr()がフォールバックするので範囲チェックは行わない
	if (!m_isSingleChartItem && (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx))
	{
		Logger << U"[ksm warning] SelectMenuSongItem::decide: Difficulty index out of range (difficultyIdx:{}, fullPath:'{}')"_fmt(difficultyIdx, m_fullPath);
		return;
	}

	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		Logger << U"[ksm warning] SelectMenuSongItem::decide: pChartInfo is null (difficultyIdx:{}, fullPath:'{}')"_fmt(difficultyIdx, m_fullPath);
		return;
	}
	const FilePath chartFilePath = FilePath{ pChartInfo->chartFilePath() };

	// 譜面ファイルの存在チェック
	if (!FileSystem::Exists(chartFilePath))
	{
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorChartFileNotFound), MessageBoxStyle::Error);
		return;
	}

	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::No, none);
}

void SelectMenuSongItem::decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx)
{
	// 単一譜面項目の場合はdifficultyIdxに関係なくchartInfoPtr()がフォールバックするので範囲チェックは行わない
	if (!m_isSingleChartItem && (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx))
	{
		Logger << U"[ksm warning] SelectMenuSongItem::decideAutoPlay: Difficulty index out of range (difficultyIdx:{}, fullPath:'{}')"_fmt(difficultyIdx, m_fullPath);
		return;
	}

	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		Logger << U"[ksm warning] SelectMenuSongItem::decideAutoPlay: pChartInfo is null (difficultyIdx:{}, fullPath:'{}')"_fmt(difficultyIdx, m_fullPath);
		return;
	}
	const FilePath chartFilePath = FilePath{ pChartInfo->chartFilePath() };

	// 譜面ファイルの存在チェック
	if (!FileSystem::Exists(chartFilePath))
	{
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorChartFileNotFound), MessageBoxStyle::Error);
		return;
	}

	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::Yes, none);
}

const SelectChartInfo* SelectMenuSongItem::chartInfoForSingleChartItem() const
{
	for (int32 i = 0; i < kNumDifficulties; ++i)
	{
		if (m_chartInfos[i] != nullptr)
		{
			return m_chartInfos[i].get();
		}
	}
	return nullptr;
}

const SelectChartInfo* SelectMenuSongItem::chartInfoPtr(int difficultyIdx, FallbackForSingleChartYN fallbackForSingleChart) const
{
	// 単一譜面項目の場合は、fallbackForSingleChartがYesならdifficultyIdxに関係なく保持している譜面情報を返す
	if (m_isSingleChartItem && fallbackForSingleChart)
	{
		return chartInfoForSingleChartItem();
	}

	if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
	{
		assert(false && "difficultyIdx out of range!");
		return nullptr;
	}

	return m_chartInfos[difficultyIdx].get();
}

void SelectMenuSongItem::setCanvasParamsCenter([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", true },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
	});

	// 各難易度の存在有無とレベルを設定
	setDifficultyLevelDisplayParams(canvas);

	// 選択中の難易度の情報を設定
	const SelectChartInfo* pChartInfo = nullptr;
	if (m_isSingleChartItem)
	{
		pChartInfo = chartInfoForSingleChartItem();
		if (pChartInfo != nullptr)
		{
			difficultyIdx = pChartInfo->difficultyIdx();
		}
	}
	else
	{
		if (0 <= difficultyIdx && difficultyIdx < kNumDifficulties)
		{
			pChartInfo = m_chartInfos[difficultyIdx].get();
		}
	}

	if (pChartInfo != nullptr)
	{
		const HighScoreInfo& highScoreInfo = pChartInfo->highScoreInfo();

		const GaugeType gaugeType = RuntimeConfig::GetGaugeType();

		const FilePath jacketPath = pChartInfo->jacketFilePath();
		const FilePath iconPath = pChartInfo->iconFilePath();

		const FilePath titleImgPath = pChartInfo->titleImgFilePath();
		const bool hasTitleImg = !titleImgPath.isEmpty() && FileSystem::IsFile(titleImgPath);
		const FilePath artistImgPath = pChartInfo->artistImgFilePath();
		const bool hasArtistImg = !artistImgPath.isEmpty() && FileSystem::IsFile(artistImgPath);

		canvas.setSubCanvasParamValuesByTag(U"center", {
			{ U"title", hasTitleImg ? U"" : pChartInfo->title() },
			{ U"titleImgFilePath", titleImgPath },
			{ U"artist", hasArtistImg ? U"" : pChartInfo->artist() },
			{ U"artistImgFilePath", artistImgPath },
			{ U"bpm", pChartInfo->dispBPM() },
			{ U"jacketAuthor", pChartInfo->jacketAuthor() },
			{ U"information", pChartInfo->information() },
			{ U"chartAuthor", pChartInfo->chartAuthor() },
			{ U"difficultyCursorState", U"difficulty{}"_fmt(difficultyIdx) },
			{ U"medalIndex", static_cast<int32>(highScoreInfo.medal()) },
			{ U"highScoreGradeIndex", static_cast<int32>(highScoreInfo.grade(gaugeType)) },
			{ U"highScore", U"{:08d}"_fmt(highScoreInfo.score(gaugeType)) },
			{ U"gaugePercentage", ToString(highScoreInfo.percent(gaugeType)) },
			{ U"jacketFilePath", jacketPath },
			{ U"jacketActive", !jacketPath.isEmpty() && FileSystem::IsFile(jacketPath) },
			{ U"iconFilePath", iconPath },
			{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
		});
	}
	else
	{
		throw Error{ U"Invalid difficultyIdx: {} (fullPath={})"_fmt(difficultyIdx, m_fullPath) };
	}
}

void SelectMenuSongItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const
{
	// 中央で選択中の難易度がこの曲に存在するかチェック
	// (difficultyIdxには中央の項目の代替カーソル値が適用済み)
	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);

	// 曲名・アーティスト名・ジャケット画像は常に描画するため、代替カーソル値を使用して取得
	const int32 altDifficultyIdx = SelectDifficultyMenu::GetAlternativeCursor(difficultyIdx,
		[this](int32 idx)
		{
			return chartInfoPtr(idx) != nullptr;
		});
	const SelectChartInfo* pAltChartInfo = chartInfoPtr(altDifficultyIdx);
	if (pAltChartInfo == nullptr)
	{
		assert(false && "SelectMenuSongItem::setCanvasParamsTopBottom: pAltChartInfo is null");
		return;
	}

	// レベルとメダルは、中央で選択中の難易度がこの曲に存在する場合のみ表示
	// (単一譜面の場合は常に表示)
	int32 levelIndex = -1; // -1は非表示
	int32 medalIndex = -1; // -1は非表示
	int32 highScoreGradeIndex = 0; // グレードは「-」表示にするため0
	int32 gaugePercentage = 0;
	const SelectChartInfo* pDisplayChartInfo = m_isSingleChartItem ? pAltChartInfo : pChartInfo;
	if (pDisplayChartInfo != nullptr)
	{
		const HighScoreInfo& highScoreInfo = pDisplayChartInfo->highScoreInfo();
		const GaugeType gaugeType = RuntimeConfig::GetGaugeType();
		levelIndex = pDisplayChartInfo->level() - 1;
		medalIndex = static_cast<int32>(highScoreInfo.medal());
		highScoreGradeIndex = static_cast<int32>(highScoreInfo.grade(gaugeType));
		gaugePercentage = highScoreInfo.percent(gaugeType);
	}

	const FilePath jacketPath = pAltChartInfo->jacketFilePath();
	const FilePath iconPath = pAltChartInfo->iconFilePath();

	const FilePath titleImgPath = pAltChartInfo->titleImgFilePath();
	const bool hasTitleImg = !titleImgPath.isEmpty() && FileSystem::IsFile(titleImgPath);
	const FilePath artistImgPath = pAltChartInfo->artistImgFilePath();
	const bool hasArtistImg = !artistImgPath.isEmpty() && FileSystem::IsFile(artistImgPath);

	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", true },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", hasTitleImg ? U"" : pAltChartInfo->title() },
		{ U"titleImgFilePath", titleImgPath },
		{ U"artist", hasArtistImg ? U"" : pAltChartInfo->artist() },
		{ U"artistImgFilePath", artistImgPath },
		{ U"levelIndex", levelIndex },
		{ U"medalIndex", medalIndex },
		{ U"highScoreGradeIndex", highScoreGradeIndex },
		{ U"gaugePercentage", ToString(gaugePercentage) },
		{ U"jacketFilePath", jacketPath },
		{ U"jacketActive", !jacketPath.isEmpty() && FileSystem::IsFile(jacketPath) },
		{ U"iconFilePath", iconPath },
		{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
	});
}

void SelectMenuSongItem::showInFileManager(int32 difficultyIdx) const
{
	// 選択中の難易度の譜面ファイルを取得
	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		return;
	}

	// エクスプローラで譜面ファイルを選択状態で開く
	System::ShowInFileManager(pChartInfo->chartFilePath());
}

Optional<HighScoreInfo> SelectMenuSongItem::highScoreInfo(int32 difficultyIdx) const
{
	const SelectChartInfo* chartInfo = chartInfoPtr(difficultyIdx);
	if (chartInfo == nullptr)
	{
		return none;
	}
	return chartInfo->highScoreInfo();
}

Optional<String> SelectMenuSongItem::relativePathToCopy(int32 difficultyIdx) const
{
	const SelectChartInfo* chartInfo = chartInfoPtr(difficultyIdx);
	if (chartInfo == nullptr)
	{
		return none;
	}
	String relativePath = FsUtils::RelativePathFromSongsDir(chartInfo->chartFilePath());
	relativePath.replace(U'\\', U'/');
	return relativePath;
}

bool SelectMenuSongItem::handleDifficultyChange(
	const SelectMenuEventContext& context,
	int32 currentDifficultyIdx,
	int32 delta) const
{
	if (m_isSingleChartItem)
	{
		return false;
	}

	// 入力方向に存在する次の難易度を探す
	if (delta > 0)
	{
		for (int32 i = currentDifficultyIdx + 1; i < kNumDifficulties; ++i)
		{
			if (m_chartInfos[i] != nullptr)
			{
				context.fnChangeDifficulty(i);
				return true;
			}
		}
	}
	else if (delta < 0)
	{
		for (int32 i = currentDifficultyIdx - 1; i >= 0; --i)
		{
			if (m_chartInfos[i] != nullptr)
			{
				context.fnChangeDifficulty(i);
				return true;
			}
		}
	}

	return false;
}

void SelectMenuSongItem::setDifficultyLevelDisplayParams(noco::Canvas& canvas) const
{
	for (int32 i = 0; i < kNumDifficulties; ++i)
	{
		const bool exists = m_chartInfos[i] != nullptr;
		const int32 levelIndex = exists ? m_chartInfos[i]->level() - 1 : -1;
		canvas.setSubCanvasParamValuesByTag(U"center", {
			{ U"difficulty{}Enabled"_fmt(i), exists },
			{ U"difficulty{}LevelIndex"_fmt(i), levelIndex },
			{ U"difficulty{}StyleState"_fmt(i), U"" },
		});
	}
}
