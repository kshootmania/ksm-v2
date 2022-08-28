#pragma once
#include "select_assets.hpp"
#include "select_menu_item.hpp"

struct SelectMenuItemTextures
{
	Texture center;
	Texture upperHalf;
	Texture lowerHalf;
};

enum class SelectMenuShakeDirection
{
	kUnspecified,
	kUp,
	kDown,
};

class SelectMenuGraphics
{
private:
	const SelectMenuItemTextures m_songItemTextures;
	const SelectMenuItemTextures m_dirItemTextures;
	const SelectMenuItemTextures m_subDirItemTextures;

	static constexpr int32 kNumDisplayItems = 8;
	static constexpr int32 kNumUpperHalfItems = kNumDisplayItems / 2;
	static constexpr int32 kNumLowerHalfItems = kNumDisplayItems - kNumUpperHalfItems - 1;

	RenderTexture m_centerItem;
	Array<RenderTexture> m_upperHalfItems;
	Array<RenderTexture> m_lowerHalfItems;

	// TODO: Use FontAsset class instead
	// TODO: Use transform instead of using font size directly
	const Font m_fontLL;
	const Font m_fontL;
	const Font m_fontM;
	const Font m_fontS;
	const Font m_fontSS;
	const Font m_fontSSS;

	void refreshCenterMenuItem(const SelectMenuItem& item, int32 difficultyIdx) const;

	void refreshUpperLowerMenuItem(const RenderTexture& target, const SelectMenuItem& item, int32 difficultyIdx, bool isUpperHalf) const;

public:
	enum RefreshType
	{
		kAll,
		kCursorUp,
		kCursorDown,
	};

	SelectMenuGraphics();

	void refresh(const ArrayWithLinearMenu<SelectMenuItem>& menu, int32 difficultyIdx, RefreshType type);

	void draw(const Vec2& shakeVec) const;
};
