#pragma once
#include "select_assets.hpp"
#include "graphics/number_texture_font.hpp"

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

	Font font = AssetManagement::SystemFont();
	Font fontBold = AssetManagement::SystemFontBold();

	TiledTexture songLevelNumberTexture
	{
		TextureAsset(SelectTexture::kSongLevelNumber),
		TiledTextureSizeInfo
		{
			.row = kNumLevels,
			.sourceSize = { 150, 120 },
		}
	};

	TiledTexture highScoreMedalTexture
	{
		TextureAsset(SelectTexture::kHighScoreMedalTexture),
		TiledTextureSizeInfo
		{
			.row = static_cast<int32>(Medal::kNumMedals),
			.sourceSize = { 188, 78 },
		}
	};

	NumberTextureFont highScoreNumberTextureFont{ SelectTexture::kHighScoreNumberTextureFont, { 136, 120 } };

	TiledTexture highScoreGradeTexture
	{
		TextureAsset(SelectTexture::kHighScoreGradeTexture),
		TiledTextureSizeInfo
		{
			.row = static_cast<int32>(Grade::kNumGrades),
			.sourceSize = { 64, 64 },
		}
	};
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

	Optional<int32> m_prevDifficultyIdx = none;

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
