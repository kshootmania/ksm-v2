#pragma once
#include "texture_font_text_layout.hpp"

using ZeroPaddingYN = YesNo<struct ZeroPaddingYN_tag>;

class NumberTextureFont
{
private:
	const TiledTexture m_tiledTexture;

public:
	enum class Align
	{
		Center,
		Left,
		Right,
	};

	explicit NumberTextureFont(StringView textureAssetKey, const Size& sourceSize = TiledTextureSizeInfo::kAutoDetectSize, const Point& sourceOffset = Point::Zero());

	/// @brief 描画
	/// @param textLayout レイアウト
	/// @param position 描画先のアンカー座標
	/// @param number 描画する数字
	/// @param zeroPadding ゼロパディング(0埋め)するかどうか
	void draw(const TextureFontTextLayout& textLayout, const Vec2& position, int32 number, ZeroPaddingYN zeroPadding) const;

	/// @brief 描画
	/// @param grid グリッド
	/// @param offsetIndexFromBack 起点インデックス(末尾を起点とした先頭方向へのインデックスで指定)
	/// @param position 描画先のアンカー座標
	/// @param number 描画する数字
	/// @remarks こちらの関数はグリッド内の一部のみに数字を描画したい場合に使う。ゼロパディング使用不可
	void draw(const TextureFontTextLayout::Grid& grid, int32 offsetIndexFromBack, int32 number) const;
};
