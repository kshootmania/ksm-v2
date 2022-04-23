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

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, bool zeroPadding, Align align) const
{
	draw(position, number, numPaddingDigits, m_scaledSize.x, zeroPadding, align);
}

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, double diffX, bool zeroPadding, Align align) const
{
	// Note: If align is kRightAlign and the number of digits exceeds numDigits, the exceeded digits are drawn to the left of the position.
	const int32 numDigits = (align == kLeftAlign) ? Max(static_cast<int32>(Log10(number) + 1.0), numPaddingDigits) : numPaddingDigits;
	int32 digitCount = 0;
	do
	{
		const int32 x = diffX * (numDigits - digitCount - 1);
		m_tiledTexture(number % 10).resized(m_scaledSize).draw(position + Vec2::Right(x));
		++digitCount;
		number /= 10;
	} while (number > 0);

	if (zeroPadding)
	{
		const int32 n = numPaddingDigits - digitCount;
		for (int32 i = 0; i < n; ++i)
		{
			const int32 x = diffX * (numDigits - digitCount - 1);
			m_tiledTexture(0).resized(m_scaledSize).draw(position + Vec2::Right(x));
			++digitCount;
		}
	}
}
