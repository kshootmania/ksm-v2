#include "font_utils.hpp"

namespace FontUtils
{
	void DrawTextCenterWithFitWidth(const DrawableText& drawableText, int32 largeFontSize, int32 smallFontSize, const Rect& rect, ColorF color)
	{
		//rect.drawFrame(4, 0, Palette::Yellow);

		const Vec2 pos = rect.center();
		const double maxWidth = rect.w;
		const double largeTextWidth = drawableText.region(largeFontSize).w;
		if (largeTextWidth <= maxWidth * 2 / 3)
		{
			drawableText.drawAt(largeFontSize, pos, color);
			return;
		}
		const double smallTextWidth = drawableText.region(smallFontSize).w;
		if (smallTextWidth <= maxWidth)
		{
			drawableText.drawAt(smallFontSize, pos, color);
			return;
		}

		const double scale = maxWidth / smallTextWidth;
		drawableText.drawAt(smallFontSize * scale, pos, color);
	}

	void DrawTextLeftWithFitWidth(const DrawableText& drawableText, int32 largeFontSize, int32 smallFontSize, const Rect& rect, ColorF color, double shiftYForSmallScale)
	{
		//rect.drawFrame(4, 0, Palette::Yellow);

		const Vec2 pos = rect.leftCenter();
		const double maxWidth = rect.w;
		const double largeTextWidth = drawableText.region(largeFontSize).w;
		if (largeTextWidth <= maxWidth * 2 / 3)
		{
			drawableText.draw(largeFontSize, Arg::leftCenter = pos, color);
			return;
		}
		const double smallTextWidth = drawableText.region(smallFontSize).w;
		if (smallTextWidth <= maxWidth)
		{
			drawableText.draw(smallFontSize, Arg::leftCenter = pos, color);
			return;
		}

		const double scale = maxWidth / smallTextWidth;
		const Vec2 shiftedPos = scale < 0.5 ? pos.movedBy(0, shiftYForSmallScale) : pos;
		drawableText.draw(smallFontSize * scale, Arg::leftCenter = shiftedPos, color);
	}
}
