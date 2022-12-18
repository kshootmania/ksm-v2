#pragma once

class NumberFontTexture
{
private:
	const TiledTexture m_tiledTexture;
	const SizeF m_scaledSize;

public:
	enum Align
	{
		kLeftAlign,
		kRightAlign,
	};

	NumberFontTexture(StringView textureAssetKey, const SizeF& scaledSize, const Size& sourceSize = TiledTextureSizeInfo::kAutoDetectSize);

	void draw(const Vec2& position, int32 number, int32 numPaddingDigits, bool zeroPadding, Align align = kRightAlign) const;

	void draw(const Vec2& position, int32 number, int32 numPaddingDigits, double diffX, bool zeroPadding, Align align = kRightAlign) const;
};
