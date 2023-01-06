#include "texture_font_text_layout.hpp"

TextureFontTextLayout::Grid::Grid(const Vec2& lastLetterPosition, const Vec2& diffPosition, const SizeF& letterSize, int32 numLettersAfterPadding)
	: m_lastLetterPosition(lastLetterPosition)
	, m_diffPosition(diffPosition)
	, m_letterSize(letterSize)
	, m_numLettersAfterPadding(numLettersAfterPadding)
{
}

RectF TextureFontTextLayout::Grid::fromFront(int32 index) const
{
	return fromBack(m_numLettersAfterPadding - 1 - index);
}

RectF TextureFontTextLayout::Grid::fromBack(int32 index) const
{
	const Vec2 position = m_lastLetterPosition + m_diffPosition * index;
	return RectF{ position, m_letterSize };
}

int32 TextureFontTextLayout::Grid::numLettersAfterPadding() const
{
	return m_numLettersAfterPadding;
}

TextureFontTextLayout::TextureFontTextLayout(const SizeF& letterSize, Align align, int32 numMaxPaddingLetters)
	: TextureFontTextLayout(letterSize, align, numMaxPaddingLetters, letterSize.x)
{
}

TextureFontTextLayout::TextureFontTextLayout(const SizeF& letterSize, Align align, int32 numMaxPaddingLetters, double scanX)
	: m_letterSize(letterSize)
	, m_align(align)
	, m_numMaxPaddingLetters(numMaxPaddingLetters)
	, m_scanX(scanX)
{
}

TextureFontTextLayout::Grid TextureFontTextLayout::grid(const Vec2& position, int32 numLetters) const
{
	assert(numLetters >= 0 && "numLetters must not be negative");
	assert(m_scanX >= 0.0 && "m_scanX must not be negative");
	assert(m_numMaxPaddingLetters >= 0 && "m_numMaxPaddingLetters must not be negative");
	assert(m_letterSize.x >= 0.0 && "m_letterSize.x must not be negative");

	int32 numLettersAfterPadding;
	double relativeRightAnchorX;
	switch (m_align)
	{
	case TextureFontTextLayout::Align::Center:
	{
		numLettersAfterPadding = Max(numLetters, m_numMaxPaddingLetters);
		const double width = m_scanX * (numLettersAfterPadding - 1) + m_letterSize.x;
		relativeRightAnchorX = width / 2 - m_letterSize.x;
		break;
	}

	case TextureFontTextLayout::Align::Left:
		numLettersAfterPadding = Max(numLetters, m_numMaxPaddingLetters);
		relativeRightAnchorX = m_scanX * (numLettersAfterPadding - 1);
		break;

	case TextureFontTextLayout::Align::Right:
		numLettersAfterPadding = m_numMaxPaddingLetters;
		relativeRightAnchorX = m_scanX * numLettersAfterPadding - m_letterSize.x;
		break;

	default:
		assert(false && "Unknown alignment");
		numLettersAfterPadding = 0;
		relativeRightAnchorX = 0.0;
		break;
	}

	return Grid(position + Vec2::Right(relativeRightAnchorX), Vec2::Left(m_scanX), m_letterSize, numLettersAfterPadding);
}
