#include "select_menu_graphics.hpp"
#include "select_difficulty_menu.hpp"
#include "menu_item/iselect_menu_item.hpp"

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

void SelectMenuGraphics::refreshCenterMenuItem(const ISelectMenuItem& item, int32 difficultyIdx) const
{
	item.drawCenter(difficultyIdx, m_centerItem, m_menuItemGraphicAssets);
}

void SelectMenuGraphics::refreshUpperLowerMenuItem(const RenderTexture& target, const ISelectMenuItem& item, int32 difficultyIdx, bool isUpper) const
{
	item.drawUpperLower(difficultyIdx, target, m_menuItemGraphicAssets, isUpper);
}

SelectMenuGraphics::SelectMenuGraphics()
	: m_centerItem({ 766, 378 }, Color::Zero())
	, m_upperHalfItems(MakeRenderTextureArray(kNumUpperHalfItems, { 816, 228 }))
	, m_lowerHalfItems(MakeRenderTextureArray(kNumLowerHalfItems, { 816, 228 }))
{
}

void SelectMenuGraphics::refresh(const ArrayWithLinearMenu<std::unique_ptr<ISelectMenuItem>>& menu, int32 difficultyIdx, RefreshType type)
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
		refreshUpperLowerMenuItem(m_upperHalfItems[0], *menu.atCyclic(menu.cursor() - kNumUpperHalfItems), difficultyIdx, true);
		refreshUpperLowerMenuItem(m_lowerHalfItems[0], *menu.atCyclic(menu.cursor() + 1), difficultyIdx, false);
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
		refreshUpperLowerMenuItem(m_upperHalfItems[kNumUpperHalfItems - 1], *menu.atCyclic(menu.cursor() - 1), difficultyIdx, true);
		refreshUpperLowerMenuItem(m_lowerHalfItems[kNumLowerHalfItems - 1], *menu.atCyclic(menu.cursor() + kNumLowerHalfItems), difficultyIdx, false);
	}
	else
	{
		// 使い回せない場合は全描画
		for (int32 i = 0; i < kNumUpperHalfItems; ++i)
		{
			const ISelectMenuItem& item = *menu.atCyclic(menu.cursor() - kNumUpperHalfItems + i);
			refreshUpperLowerMenuItem(m_upperHalfItems[i], item, difficultyIdx, true);
		}
		for (int i = 0; i < kNumLowerHalfItems; ++i)
		{
			const ISelectMenuItem& item = *menu.atCyclic(menu.cursor() + 1 + i);
			refreshUpperLowerMenuItem(m_lowerHalfItems[i], item, difficultyIdx, false);
		}
	}

	// 中央の項目のテクスチャを更新
	{
		const ISelectMenuItem& item = *menu.cursorValue();
		refreshCenterMenuItem(item, difficultyIdx);
	}
}

void SelectMenuGraphics::draw(const Vec2& shakeVec) const
{
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
