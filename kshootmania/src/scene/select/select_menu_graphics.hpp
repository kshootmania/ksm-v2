#pragma once
#include "select_assets.hpp"

class ISelectMenuItem;

struct SelectMenuItemTextures
{
	Texture center;
	Texture upperHalf;
	Texture lowerHalf;
};

struct SelectMenuItemGraphicAssets
{
	SelectMenuItemTextures songItemTextures
	{
		.center = TextureAsset(SelectTexture::kSongCenter),
		.upperHalf = TextureAsset(SelectTexture::kSongUpperHalf),
		.lowerHalf = TextureAsset(SelectTexture::kSongLowerHalf),
	};

	SelectMenuItemTextures dirItemTextures
	{
		.center = TextureAsset(SelectTexture::kDirCenter),
		.upperHalf = TextureAsset(SelectTexture::kDirUpperHalf),
		.lowerHalf = TextureAsset(SelectTexture::kDirLowerHalf),
	};

	SelectMenuItemTextures subDirItemTextures
	{
		.center = TextureAsset(SelectTexture::kSubDirCenter),
		.upperHalf = TextureAsset(SelectTexture::kSubDirUpperHalf),
		.lowerHalf = TextureAsset(SelectTexture::kSubDirLowerHalf),
	};

	// TODO: Use FontAsset class instead
	// TODO: Use SDF instead of using font size directly
	Font fontLL{ 44, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
	Font fontL{ 38, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
	Font fontM{ 30, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
	Font fontS{ 24, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
	Font fontSS{ 19, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
	Font fontSSS{ 17, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default };
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
	const SelectMenuItemGraphicAssets m_menuItemGraphicAssets;

	static constexpr int32 kNumDisplayItems = 8;
	static constexpr int32 kNumUpperHalfItems = kNumDisplayItems / 2;
	static constexpr int32 kNumLowerHalfItems = kNumDisplayItems - kNumUpperHalfItems - 1;

	RenderTexture m_centerItem;
	Array<RenderTexture> m_upperHalfItems;
	Array<RenderTexture> m_lowerHalfItems;

	void refreshCenterMenuItem(const ISelectMenuItem& item, int32 difficultyIdx) const;

	void refreshUpperLowerMenuItem(const RenderTexture& target, const ISelectMenuItem& item, int32 difficultyIdx, bool isUpper) const;

public:
	enum RefreshType
	{
		kAll,
		kCursorUp,
		kCursorDown,
	};

	SelectMenuGraphics();

	void refresh(const ArrayWithLinearMenu<std::unique_ptr<ISelectMenuItem>>& menu, int32 difficultyIdx, RefreshType type);

	void draw(const Vec2& shakeVec) const;
};
