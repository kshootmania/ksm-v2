#pragma once

namespace FontUtils
{
	void DrawTextCenterWithFitWidth(const DrawableText& drawableText, int32 largeFontSize, int32 smallFontSize, const Rect& rect, ColorF color = Palette::White);

	void DrawTextLeftWithFitWidth(const DrawableText& drawableText, int32 largeFontSize, int32 smallFontSize, const Rect& rect, ColorF color = Palette::White, double shiftYForSmallScale = 0.0);
}
