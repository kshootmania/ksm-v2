#pragma once
#include "select_assets.hpp"

struct SelectMenuItemTextures
{
	Texture center;
	Texture upperHalf;
	Texture lowerHalf;
};

enum class SelectMenuShakeDrection
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

	static constexpr int32 kNumDisplayItems = 8;
	static constexpr int32 kNumUpperHalfItems = kNumDisplayItems / 2;
	static constexpr int32 kNumLowerHalfItems = kNumDisplayItems - kNumUpperHalfItems - 1;

	RenderTexture m_centerItem;
	Array<RenderTexture> m_upperHalfItems;
	Array<RenderTexture> m_lowerHalfItems;

	SelectMenuShakeDrection m_shakeDirection = SelectMenuShakeDrection::kUnspecified;
	Stopwatch m_shakeStopwatch;

	// TODO: Use FontAsset class instead
	const Font m_fontLL;
	const Font m_fontL;
	const Font m_fontMBold;
	const Font m_fontM;
	const Font m_fontS;

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

	void draw() const;
};
