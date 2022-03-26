#include "select_menu_graphics.hpp"
#include "select_menu_item.hpp"

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
}

void SelectMenuGraphics::drawUpperLowerMenuItem(const RenderTexture& target, const SelectMenuItem& item, int32 difficultyIdx, bool isUpperHalf) const
{
	const ScopedRenderTarget2D scopedRenderTarget(target);

	switch (item.itemType)
	{
	case SelectMenuItem::kSong:
		Shader::Copy(isUpperHalf ? m_songItemTextures.upperHalf : m_songItemTextures.lowerHalf, target);
		if (auto pInfo = dynamic_cast<SelectMenuSongItemInfo*>(item.info.get()))
		{
			if (pInfo->chartInfos[difficultyIdx].has_value())
			{
				m_fontM(pInfo->chartInfos[difficultyIdx]->title).drawAt(isUpperHalf ? Vec2{ 12 + 576 / 2, 10 + 40 / 2 } : Vec2{ 12 + 576 / 2, 118 + 40 / 2 });
			}
		}
		break;

	case SelectMenuItem::kAllFolder:
	case SelectMenuItem::kCurrentFolder:
	case SelectMenuItem::kDirectoryFolder:
		Shader::Copy(isUpperHalf ? m_dirItemTextures.upperHalf : m_dirItemTextures.lowerHalf, target);
		{
			String displayName = U"?";
			if (item.itemType == SelectMenuItem::kAllFolder)
			{
				displayName = U"     All   >>";
			}
			else if (auto pInfo = dynamic_cast<SelectMenuFolderItemInfo*>(item.info.get()))
			{
				displayName = U"     {}   >>"_fmt(pInfo->displayName);
			}
			m_fontL(displayName).drawAt(isUpperHalf ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
		}
		break;

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
	, m_fontL(40, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontM(30, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_fontS(20, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
{
}

void SelectMenuGraphics::refresh(const ArrayWithLinearMenu<SelectMenuItem>& menu, int32 difficultyIdx) const
{
	const ScopedRenderStates2D state(kBlendState);

	// Draw upper half items
	for (int i = 0; i < kNumUpperHalfItems; ++i)
	{
		const SelectMenuItem& item = menu.atCyclic(menu.cursor() - kNumUpperHalfItems + i);
		drawUpperLowerMenuItem(m_upperHalfItems[i], item, difficultyIdx, true);
	}

	// Draw lower half items
	for (int i = 0; i < kNumLowerHalfItems; ++i)
	{
		const SelectMenuItem& item = menu.atCyclic(menu.cursor() + 1 + i);
		drawUpperLowerMenuItem(m_lowerHalfItems[i], item, difficultyIdx, false);
	}
}

void SelectMenuGraphics::draw() const
{
	using namespace ScreenUtils;

	// Draw upper half items
	for (int32 i = 0; i < std::ssize(m_upperHalfItems); ++i)
	{
		const int32 distance = kNumUpperHalfItems - i;

		m_upperHalfItems[i].resized(ScaledL(1632, 456) * 3 / 5).draw(Scaled(85 + distance * 20 + distance * distance * 10 / 4 - 7, i * (30 + i * 2 / 3) - 3) + LeftMarginVec());
	}

	// Draw lower half items
	for (int32 i = static_cast<int32>(std::ssize(m_lowerHalfItems) - 1); i >= 0; --i)
	{
		const int32 distance = i + 1;
		m_lowerHalfItems[i].resized(ScaledL(1632, 456) * 3 / 5).draw(Scaled(85 + distance * 20 + distance * distance * 13 / 4 - 7, 139 + (i + kNumUpperHalfItems + 1) * (34 - i * 4 / 9) - 3) + LeftMarginVec());
	}

	// Draw center item
	ScaledL(m_centerItem).draw();
}
