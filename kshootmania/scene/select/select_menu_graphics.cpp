#include "select_menu_graphics.hpp"
#include "select_menu_item.hpp"
#include "select_difficulty_menu.hpp"

namespace
{
	// Set srcAlpha to One in order to use alpha in RenderTexture
	constexpr BlendState kBlendState(
		true,
		Blend::SrcAlpha,
		Blend::InvSrcAlpha,
		BlendOp::Add,
		Blend::One);

	Array<RenderTexture> MakeRenderTextureArray(int32 arrayLength, const Size& textureSize)
	{
		Array<RenderTexture> array;
		for (int32 i = 0; i < arrayLength; ++i)
		{
			array.emplace_back(textureSize, Color::Zero());
		}
		return array;
	}

	String FolderItemDisplayName(const SelectMenuItem& item, bool isCenter)
	{
		StringView displayName = U"?";
		if (item.itemType == SelectMenuItem::kAllFolder)
		{
			displayName = U"All";
		}
		else if (auto pInfo = dynamic_cast<SelectMenuFolderItemInfo*>(item.info.get()))
		{
			displayName = pInfo->displayName;
		}

		if (isCenter)
		{
			if (item.itemType == SelectMenuItem::kCurrentFolder)
			{
				return U"<<   " + displayName + U"   <<";
			}
			else
			{
				return U">>   " + displayName + U"   >>";
			}
		}
		else
		{
			if (item.itemType == SelectMenuItem::kCurrentFolder)
			{
				return U"<<   " + displayName + U"     ";
			}
			else
			{
				return U"     " + displayName + U"   >>";
			}
		}
	}

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

void SelectMenuGraphics::refreshCenterMenuItem(const SelectMenuItem& item, int32 difficultyIdx) const
{
	const ScopedRenderTarget2D scopedRenderTarget(m_centerItem);

	switch (item.itemType)
	{
	case SelectMenuItem::kSong:
		Shader::Copy(m_songItemTextures.center, m_centerItem);
		if (auto pInfo = dynamic_cast<SelectMenuSongItemInfo*>(item.info.get())) [[likely]]
		{
			if (pInfo->chartInfos[difficultyIdx].has_value())
			{
				const auto& chartInfo = *(pInfo->chartInfos[difficultyIdx]);

				// Title
				for (int32 i = 0; i < 2; ++i)
				{
					// Draw twice to make the font slightly bold
					// (Using a bold typeface makes the font too bold compared to HSP's Artlet2D)
					// TODO: Maybe fonts with bold glyphs (e.g. Arial) do not have this problem (unconfirmed), so simply use bold for them.
					m_fontM(chartInfo.title).drawAt(Vec2{ 16 + i + 462 / 2, 11 + 36 / 2 });
				}

				// Artist
				m_fontS(chartInfo.artist).drawAt(Vec2{ 16 + 462 / 2, 48 + 36 / 2 });

				// Jacket
				DrawJacketImage(chartInfo.jacketFilePath, { 492, 22 }, { 254, 254 });

				// Jacket author (Illustrated by)
				m_fontSSS(chartInfo.jacketAuthor).draw(Arg::leftCenter = Vec2{ 630, 290 + 18 / 2 }, Palette::Black);

				// Chart author (Effected by)
				m_fontSS(chartInfo.chartAuthor).draw(Arg::leftCenter = Vec2{ 154, 293 + 22 / 2 });
			}
		}
		break;

	case SelectMenuItem::kAllFolder:
	case SelectMenuItem::kCurrentFolder:
	case SelectMenuItem::kDirectoryFolder:
		Shader::Copy(m_dirItemTextures.center, m_centerItem);
		m_fontLL(FolderItemDisplayName(item, true)).drawAt(Vec2{ 16 + 740 / 2, 135 + 102 / 2 });
		break;

		// TODO: Other types

	default:
		m_centerItem.clear(Color::Zero());
		break;
	}
}

void SelectMenuGraphics::refreshUpperLowerMenuItem(const RenderTexture& target, const SelectMenuItem& item, int32 difficultyIdx, bool isUpperHalf) const
{
	const ScopedRenderTarget2D scopedRenderTarget(target);

	switch (item.itemType)
	{
	case SelectMenuItem::kSong:
		Shader::Copy(isUpperHalf ? m_songItemTextures.upperHalf : m_songItemTextures.lowerHalf, target);
		if (const auto pInfo = dynamic_cast<SelectMenuSongItemInfo*>(item.info.get())) [[likely]]
		{
			const int32 altDifficultyIdx = SelectDifficultyMenu::GetAlternativeCursor(difficultyIdx,
				[pInfo](int32 idx)
				{
					return 0 <= idx && std::cmp_less(idx, pInfo->chartInfos.size()) && pInfo->chartInfos[idx].has_value();
				});

			if (altDifficultyIdx >= 0) [[likely]]
			{
				const auto& chartInfo = *(pInfo->chartInfos[altDifficultyIdx]);

				// 曲名を描画
				for (int32 i = 0; i < 2; ++i)
				{
					// Draw twice to make the font slightly bold
					// (Using a bold typeface makes the font too bold compared to HSP's Artlet2D)
					m_fontM(chartInfo.title).drawAt(isUpperHalf ? Vec2{ 12 + i + 576 / 2, 8 + 40 / 2 } : Vec2{ 12 + i + 576 / 2, 116 + 40 / 2 });
				}

				// アーティスト名を描画
				m_fontS(chartInfo.artist).drawAt(isUpperHalf ? Vec2{ 230 + 354 / 2, 67 + 40 / 2 } : Vec2{ 230 + 354 / 2, 171 + 40 / 2 });

				// ジャケット画像を描画
				DrawJacketImage(chartInfo.jacketFilePath, { 600, 10 }, { 208, 208 });
			}
		}
		break;

	case SelectMenuItem::kAllFolder:
	case SelectMenuItem::kCurrentFolder:
	case SelectMenuItem::kDirectoryFolder:
		Shader::Copy(isUpperHalf ? m_dirItemTextures.upperHalf : m_dirItemTextures.lowerHalf, target);
		m_fontL(FolderItemDisplayName(item, false)).drawAt(isUpperHalf ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
		break;

		// TODO: Other types

	default:
		target.clear(Color::Zero());
		break;
	}
}

SelectMenuGraphics::SelectMenuGraphics()
	: m_songItemTextures{
		.center = TextureAsset(SelectTexture::kSongCenter),
		.upperHalf = TextureAsset(SelectTexture::kSongUpperHalf),
		.lowerHalf = TextureAsset(SelectTexture::kSongLowerHalf) }
	, m_dirItemTextures{
		.center = TextureAsset(SelectTexture::kDirCenter),
		.upperHalf = TextureAsset(SelectTexture::kDirUpperHalf),
		.lowerHalf = TextureAsset(SelectTexture::kDirLowerHalf) }
	, m_centerItem({ 766, 378 }, Color::Zero())
	, m_upperHalfItems(MakeRenderTextureArray(kNumUpperHalfItems, { 816, 228 }))
	, m_lowerHalfItems(MakeRenderTextureArray(kNumLowerHalfItems, { 816, 228 }))
	, m_fontLL(44, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontL(38, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontM(30, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontS(24, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontSS(19, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontSSS(17, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
{
}

void SelectMenuGraphics::refresh(const ArrayWithLinearMenu<SelectMenuItem>& menu, int32 difficultyIdx, RefreshType type)
{
	const ScopedRenderStates2D state(kBlendState);

	// 中央以外の項目のテクスチャを更新
	if (type == kCursorUp)
	{
		// 上移動の場合、一番上と中央の1つ下以外は既に描画済みのものがあるので使い回す
		// (使い回す際、swapする方がShader::Copy()より圧倒的に速いのでswapを採用)
		for (int32 i = kNumUpperHalfItems - 1; i > 0; --i)
		{
			m_upperHalfItems[i].swap(m_upperHalfItems[i - 1]);
		}
		for (int32 i = kNumLowerHalfItems - 1; i > 0; --i)
		{
			m_lowerHalfItems[i].swap(m_lowerHalfItems[i - 1]);
		}
		refreshUpperLowerMenuItem(m_upperHalfItems[0], menu.atCyclic(menu.cursor() - kNumUpperHalfItems), difficultyIdx, true);
		refreshUpperLowerMenuItem(m_lowerHalfItems[0], menu.atCyclic(menu.cursor() + 1), difficultyIdx, false);
	}
	else if (type == kCursorDown)
	{
		// 下移動の場合、一番下と中央の1つ上以外は既に描画済みのものがあるので使い回す
		// (使い回す際、swapする方がShader::Copy()より圧倒的に速いのでswapを採用)
		for (int32 i = 0; i < kNumUpperHalfItems - 1; ++i)
		{
			m_upperHalfItems[i].swap(m_upperHalfItems[i + 1]);
		}
		for (int32 i = 0; i < kNumLowerHalfItems - 1; ++i)
		{
			m_lowerHalfItems[i].swap(m_lowerHalfItems[i + 1]);
		}
		refreshUpperLowerMenuItem(m_upperHalfItems[kNumUpperHalfItems - 1], menu.atCyclic(menu.cursor() - 1), difficultyIdx, true);
		refreshUpperLowerMenuItem(m_lowerHalfItems[kNumLowerHalfItems - 1], menu.atCyclic(menu.cursor() + kNumLowerHalfItems), difficultyIdx, false);
	}
	else
	{
		// 使い回せない場合は全描画
		for (int32 i = 0; i < kNumUpperHalfItems; ++i)
		{
			const SelectMenuItem& item = menu.atCyclic(menu.cursor() - kNumUpperHalfItems + i);
			refreshUpperLowerMenuItem(m_upperHalfItems[i], item, difficultyIdx, true);
		}
		for (int i = 0; i < kNumLowerHalfItems; ++i)
		{
			const SelectMenuItem& item = menu.atCyclic(menu.cursor() + 1 + i);
			refreshUpperLowerMenuItem(m_lowerHalfItems[i], item, difficultyIdx, false);
		}
	}

	// 中央の項目のテクスチャを更新
	{
		const SelectMenuItem& item = menu.at(menu.cursor());
		refreshCenterMenuItem(item, difficultyIdx);
	}
}

void SelectMenuGraphics::draw(const Vec2& shakeVec) const
{
	using namespace ScreenUtils;

	// 上半分の項目を描画
	for (int32 i = 0; i < std::ssize(m_upperHalfItems); ++i)
	{
		const int32 distance = kNumUpperHalfItems - i;

		m_upperHalfItems[i].resized(ScaledL(1632, 456) * 3 / 5).draw(Scaled(85 + distance * 20 + distance * distance * 10 / 4 - 7, i * (30 + i * 2 / 3) - 3) + LeftMarginVec());
	}

	// 下半分の項目を描画
	for (int32 i = static_cast<int32>(std::ssize(m_lowerHalfItems) - 1); i >= 0; --i)
	{
		const int32 distance = i + 1;
		m_lowerHalfItems[i].resized(ScaledL(1632, 456) * 3 / 5).draw(Scaled(85 + distance * 20 + distance * distance * 13 / 4 - 7, 139 + (i + kNumUpperHalfItems + 1) * (34 - i * 4 / 9) - 3) + LeftMarginVec());
	}

	// 中央の項目を描画
	m_centerItem.resized(Scaled(450, 222)).draw(Scaled(65, 128) + LeftMarginVec() + shakeVec);
}
