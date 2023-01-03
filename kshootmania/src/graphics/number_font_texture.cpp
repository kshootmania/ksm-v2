#include "number_font_texture.hpp"

namespace
{
	double OffsetX(int32 number, int32 numPaddingDigits, double scanX, NumberFontTexture::Align align, double scaledTextureWidth)
	{
		assert(scanX >= 0.0 && "scanX must not be negative");
		assert(scaledTextureWidth >= 0.0 && "scaledTextureWidth must not be negative");

		switch (align)
		{
		case NumberFontTexture::Align::Center:
		{
			const int32 numDigits = Max(MathUtils::NumDigits(number), numPaddingDigits);
			assert(numDigits >= 1);
			const double width = scanX * (numDigits - 1) + scaledTextureWidth;
			return width / 2 - scaledTextureWidth;
		}

		case NumberFontTexture::Align::Left:
		{
			const int32 numDigits = Max(MathUtils::NumDigits(number), numPaddingDigits);
			assert(numDigits >= 1);
			return scanX * (numDigits - 1);
		}

		case NumberFontTexture::Align::Right:
			assert(numPaddingDigits >= 0);
			return scanX * numPaddingDigits - scaledTextureWidth;

		default:
			assert(false && "Unknown alignment");
			return 0.0;
		}
	}
}

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

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, ZeroPaddingYN zeroPadding, Align align) const
{
	draw(position, number, numPaddingDigits, m_scaledSize.x, zeroPadding, align);
}

void NumberFontTexture::draw(const Vec2& position, int32 number, int32 numPaddingDigits, double scanX, ZeroPaddingYN zeroPadding, Align align) const
{
	const double offsetX = OffsetX(number, numPaddingDigits, scanX, align, m_scaledSize.x);
	int32 digitCount = 0;
	do
	{
		const double x = offsetX - scanX * digitCount;
		m_tiledTexture(number % 10).resized(m_scaledSize).draw(position + Vec2::Right(x));
		++digitCount;
		number /= 10;
	} while (number > 0);

	if (zeroPadding)
	{
		const int32 n = numPaddingDigits - digitCount;
		for (int32 i = 0; i < n; ++i)
		{
			const double x = offsetX - scanX * digitCount;
			m_tiledTexture(0).resized(m_scaledSize).draw(position + Vec2::Right(x));
			++digitCount;
		}
	}
}
