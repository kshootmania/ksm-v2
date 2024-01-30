#include "select_menu_song_item.hpp"
#include "graphics/font_utils.hpp"
#include "scene/select/select_difficulty_menu.hpp"
#include "scene/select/select_menu_graphics.hpp"

namespace
{
	void DrawJacketImage(FilePathView filePath, const Vec2& pos, SizeF size)
	{
		// TODO: Cache jacket image texture

		if (!FileSystem::IsFile(filePath))
		{
			return;
		}

		const Texture jacketTexture(filePath);
		if (jacketTexture.width() < jacketTexture.height())
		{
			size.x *= static_cast<double>(jacketTexture.width()) / jacketTexture.height();
		}
		else if (jacketTexture.height() < jacketTexture.width())
		{
			size.y *= static_cast<double>(jacketTexture.height()) / jacketTexture.width();
		}
		jacketTexture.resized(size).draw(pos); 
	}
}

SelectMenuSongItem::SelectMenuSongItem(const FilePath& songDirectoryPath)
{
	const Array<FilePath> chartFilePaths = FileSystem::DirectoryContents(songDirectoryPath, Recursive::No);
	for (const auto& chartFilePath : chartFilePaths)
	{
		if (FileSystem::Extension(chartFilePath) != kKSHExtension) // Note: FileSystem::Extension()は常に小文字を返すので大文字は考慮不要
		{
			continue;
		}

		auto chartInfo = std::make_unique<SelectChartInfo>(chartFilePath);

		if (chartInfo->hasError())
		{
			Logger << U"[SelectMenu] KSH Loading Error: {} ({})"_fmt(chartInfo->errorString(), chartFilePath);
			continue;
		}

		int32 difficultyIdx = chartInfo->difficultyIdx();
		if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx) [[unlikely]]
		{
			// 未知の難易度の場合は一番右の難易度にする
			Logger << U"[SelectMenu] Difficulty index out of range: {} ({})"_fmt(difficultyIdx, chartFilePath);
			difficultyIdx = kNumDifficulties - 1;
		}

		if (m_chartInfos[difficultyIdx] != nullptr) [[unlikely]]
		{
			Logger << U"[SelectMenu] Skip duplication (difficultyIdx:{}): {}"_fmt(difficultyIdx, chartFilePath);
			continue;
		}

		m_chartInfos[difficultyIdx] = std::move(chartInfo);

		m_chartExists = true;
	}
}

void SelectMenuSongItem::decide(const SelectMenuEventContext& context, int32 difficultyIdx)
{
	if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
	{
		Logger << U"[SelectMenu] Difficulty index out of range: {} ({})"_fmt(difficultyIdx, m_fullPath);
		return;
	}

	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		Logger << U"[SelectMenu] pChartInfo is null (difficultyIdx:{}): {}"_fmt(difficultyIdx, m_fullPath);
		return;
	}
	const FilePath chartFilePath = FilePath{ pChartInfo->chartFilePath() };
	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::No);
}

void SelectMenuSongItem::decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx)
{
	if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
	{
		Logger << U"[SelectMenu] Difficulty index out of range: {} ({})"_fmt(difficultyIdx, m_fullPath);
		return;
	}

	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		Logger << U"[SelectMenu] pChartInfo is null (difficultyIdx:{}): {}"_fmt(difficultyIdx, m_fullPath);
		return;
	}
	const FilePath chartFilePath = FilePath{ pChartInfo->chartFilePath() };
	context.fnMoveToPlayScene(chartFilePath, MusicGame::IsAutoPlayYN::Yes);
}

const SelectChartInfo* SelectMenuSongItem::chartInfoPtr(int difficultyIdx) const
{
	if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
	{
		assert(false && "difficultyIdx out of range!");
		return nullptr;
	}

	return m_chartInfos[difficultyIdx].get();
}

void SelectMenuSongItem::drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const
{
	Shader::Copy(assets.songItemTextures.center, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);

	const SelectChartInfo* pChartInfo = chartInfoPtr(difficultyIdx);
	if (pChartInfo == nullptr)
	{
		assert(false && "SelectMenuSongItem::drawCenter: invalid difficultyIdx");
		return;
	}

	// Title
	FontUtils::DrawTextCenterWithFitWidth(assets.fontBold(pChartInfo->title()), 27, 25, { 16, 12, 462, 36 });

	// Artist
	FontUtils::DrawTextCenterWithFitWidth(assets.font(pChartInfo->artist()), 23, 22, { 16, 48, 462, 36 });

	// Jacket
	DrawJacketImage(pChartInfo->jacketFilePath(), { 492, 22 }, { 254, 254 });

	// Jacket author (Illustrated by)
	FontUtils::DrawTextLeftWithFitWidth(assets.font(pChartInfo->jacketAuthor()), 17, 15, { 630, 290, 110, 18 }, Palette::Black, 1);

	// Chart author (Effected by)
	FontUtils::DrawTextLeftWithFitWidth(assets.font(pChartInfo->chartAuthor()), 18, 17, { 154, 293, 320, 22 }, Palette::White, 1);

	// クリアメダル・ハイスコア・グレードを描画
	const GaugeType gaugeType = GaugeType::kNormalGauge; // TODO: 現在選択中のゲージタイプを反映
	const auto& highScoreInfo = pChartInfo->highScoreInfo();
	assets.highScoreMedalTexture(static_cast<int32>(highScoreInfo.medal())).scaled(kScale2x).draw(42, 323);
	const TextureFontTextLayout highScoreTextLayout({ 17, 15 }, TextureFontTextLayout::Align::Left, 8, 18.5);
	assets.highScoreNumberTextureFont.draw(highScoreTextLayout, { 160, 342 }, highScoreInfo.score(gaugeType), ZeroPaddingYN::Yes);
	assets.highScoreGradeTexture(static_cast<int32>(highScoreInfo.grade(gaugeType))).scaled(kScale2x).draw(334, 327);
}

void SelectMenuSongItem::drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.songItemTextures.upperHalf : assets.songItemTextures.lowerHalf, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);

	const int32 altDifficultyIdx = SelectDifficultyMenu::GetAlternativeCursor(difficultyIdx,
		[this](int32 idx)
		{
			return chartInfoPtr(idx) != nullptr;
		});

	const SelectChartInfo* pChartInfo = chartInfoPtr(altDifficultyIdx);
	if (pChartInfo == nullptr)
	{
		assert(false && "SelectMenuSongItem::drawUpperLower: pChartInfo is null");
		return;
	}

	// 曲名を描画
	FontUtils::DrawTextCenterWithFitWidth(assets.fontBold(pChartInfo->title()), 28, 26, { 22, isUpper ? 10 : 117, 556, 40 });

	// アーティスト名を描画
	FontUtils::DrawTextCenterWithFitWidth(assets.font(pChartInfo->artist()), 24, 22, { 245, isUpper ? 67 : 171, 324, 40 });

	// ジャケット画像を描画
	DrawJacketImage(pChartInfo->jacketFilePath(), { 600, 10 }, { 208, 208 });

	// クリアメダル・グレードを描画
	const GaugeType gaugeType = GaugeType::kNormalGauge; // TODO: 現在選択中のゲージタイプを反映
	const auto& highScoreInfo = pChartInfo->highScoreInfo();
	assets.highScoreMedalTexture(static_cast<int32>(highScoreInfo.medal())).scaled(kScale2x).draw(70, isUpper ? 70 : 176);
	assets.highScoreGradeTexture(static_cast<int32>(highScoreInfo.grade(gaugeType))).scaled(kScale2x).draw(183, isUpper ? 75 : 181);
}
