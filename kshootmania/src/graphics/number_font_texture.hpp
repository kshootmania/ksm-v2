#pragma once

using ZeroPaddingYN = YesNo<struct ZeroPaddingYN_tag>;

class NumberFontTexture
{
private:
	const TiledTexture m_tiledTexture;
	const SizeF m_scaledSize;

public:
	enum class Align
	{
		Left,
		Right,
	};

	explicit NumberFontTexture(StringView textureAssetKey, const SizeF& scaledSize, const Size& sourceSize = TiledTextureSizeInfo::kAutoDetectSize, const Point& offset = Point::Zero());

	void draw(const Vec2& position, int32 number, int32 numPaddingDigits, ZeroPaddingYN zeroPadding, Align align = Align::Right) const;

	void draw(const Vec2& position, int32 number, int32 numPaddingDigits, double diffX, ZeroPaddingYN zeroPadding, Align align = Align::Right) const;
};
