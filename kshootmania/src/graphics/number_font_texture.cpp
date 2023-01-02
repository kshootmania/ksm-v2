#include "number_font_texture.hpp"

NumberFontTexture::NumberFontTexture(StringView textureAssetKey, const SizeF& scaledSize, const Size& sourceSize, const Point& offset)
	: m_tiledTexture(TextureAsset(textureAssetKey),
		{
			.row = 10,
			.offset = offset,
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
	// 注意:
	//   positionより左側にも描画される場合がある。
	//   具体的には、alignがkRightAlignで、かつ桁数がnumDigitsを上回った場合に、上回った分の桁がpositionより左側に描画される。
	const int32 numDigits = (align == kLeftAlign) ? Max(MathUtils::NumDigits(number), numPaddingDigits) : numPaddingDigits;
	int32 digitCount = 0;
	do
	{
		const double x = diffX * (numDigits - digitCount - 1);
		m_tiledTexture(number % 10).resized(m_scaledSize).draw(position + Vec2::Right(x));
		++digitCount;
		number /= 10;
	} while (number > 0);

	if (zeroPadding)
	{
		const int32 n = numPaddingDigits - digitCount;
		for (int32 i = 0; i < n; ++i)
		{
			const double x = diffX * (numDigits - digitCount - 1);
			m_tiledTexture(0).resized(m_scaledSize).draw(position + Vec2::Right(x));
			++digitCount;
		}
	}
}
