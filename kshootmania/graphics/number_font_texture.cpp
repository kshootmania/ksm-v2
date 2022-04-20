#include "number_font_texture.hpp"

NumberFontTexture::NumberFontTexture(StringView textureAssetKey, const SizeF& scaledSize, const Size& sourceSize)
	: m_tiledTexture(TextureAsset(textureAssetKey),
		{
			.row = 10,
			.sourceSize = sourceSize,
		})
	, m_scaledSize(scaledSize)
{
}

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, bool zeroPadding) const
{
	draw(position, number, numPaddingDigits, m_scaledSize.x, zeroPadding);
}

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, double diffX, bool zeroPadding) const
{
	// Note: If the number of digits exceeds numDigits, draw to the left of position
	int32 digitCount = 0;
	do
	{
		m_tiledTexture(number % 10).resized(m_scaledSize).draw(position + Vec2::Right(diffX * (numPaddingDigits - digitCount - 1)));
		++digitCount;
		number /= 10;
	} while (number > 0);

	if (zeroPadding)
	{
		const int32 n = numPaddingDigits - digitCount;
		for (int32 i = 0; i < n; ++i)
		{
			m_tiledTexture(0).resized(m_scaledSize).draw(position + Vec2::Right(diffX * (numPaddingDigits - digitCount - 1)));
			++digitCount;
		}
	}
}
