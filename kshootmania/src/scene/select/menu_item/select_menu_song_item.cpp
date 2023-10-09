#include "select_menu_song_item.hpp"
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
	context.fnMoveToPlayScene(chartFilePath);
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
	assets.fontBold(pChartInfo->title()).drawAt(28, Vec2{ 16 + 462 / 2, 12 + 36 / 2 });

	// Artist
	assets.font(pChartInfo->artist()).drawAt(24, Vec2{ 16 + 462 / 2, 48 + 36 / 2 });

	// Jacket
	DrawJacketImage(pChartInfo->jacketFilePath(), { 492, 22 }, { 254, 254 });

	// Jacket author (Illustrated by)
	assets.font(pChartInfo->jacketAuthor()).draw(17, Arg::leftCenter = Vec2{ 630, 290 + 18 / 2 }, Palette::Black);

	// Chart author (Effected by)
	assets.font(pChartInfo->chartAuthor()).draw(19, Arg::leftCenter = Vec2{ 154, 293 + 22 / 2 });
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
	assets.fontBold(pChartInfo->title()).drawAt(28, isUpper ? Vec2{ 12 + 576 / 2, 10 + 40 / 2 } : Vec2{ 12 + 576 / 2, 117 + 40 / 2 });

	// アーティスト名を描画
	assets.font(pChartInfo->artist()).drawAt(24, isUpper ? Vec2{ 230 + 354 / 2, 67 + 40 / 2 } : Vec2{ 230 + 354 / 2, 171 + 40 / 2 });

	// ジャケット画像を描画
	DrawJacketImage(pChartInfo->jacketFilePath(), { 600, 10 }, { 208, 208 });
}
