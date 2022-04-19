﻿#pragma once

class NumberFontTexture
{
private:
	const TiledTexture m_tiledTexture;
	const SizeF m_scaledSize;

public:
	NumberFontTexture(StringView textureAssetKey, const SizeF& scaledSize, const Size& sourceSize = TiledTextureSizeInfo::kAutoDetectSize);

	void draw(const Vec2& position, int32 number, int32 numDigits, bool zeroPadding) const;

	void draw(const Vec2& position, int32 number, int32 numDigits, double diffX, bool zeroPadding) const;
};
