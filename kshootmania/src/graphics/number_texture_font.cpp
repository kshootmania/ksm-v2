#include "number_texture_font.hpp"

NumberTextureFont::NumberTextureFont(StringView textureAssetKey, const Size& sourceSize, const Point& sourceOffset)
	: m_tiledTexture(textureAssetKey,
		{
			.row = 10,
			.sourceOffset = sourceOffset,
			.sourceSize = sourceSize,
		})
{
}

void NumberTextureFont::draw(const TextureFontTextLayout& textLayout, const Vec2& position, int32 number, ZeroPaddingYN zeroPadding) const
{
	// 桁数
	const int32 numDigits = MathUtils::NumDigits(number);

	// グリッド取得
	const auto grid = textLayout.grid(position, numDigits);

	// 描画すべき桁数
	const int32 numDigitsToDraw = zeroPadding ? grid.numLettersAfterPadding() : numDigits;

	// 下の桁から順番に描画
	for (int32 i = 0; i < numDigitsToDraw; ++i)
	{
		const RectF rect = grid.fromBack(i);
		m_tiledTexture(number % 10).resized(rect.size).draw(rect.pos);
		number /= 10;
	}
}

void NumberTextureFont::draw(const TextureFontTextLayout::Grid& grid, int32 offsetIndex, int32 number) const
{
	// 描画すべき桁数
	const int32 numDigitsToDraw = MathUtils::NumDigits(number);

	// 下の桁から順番に描画
	for (int32 i = 0; i < numDigitsToDraw; ++i)
	{
		const RectF rect = grid.fromBack(offsetIndex + i);
		m_tiledTexture(number % 10).resized(rect.size).draw(rect.pos);
		number /= 10;
	}
}
