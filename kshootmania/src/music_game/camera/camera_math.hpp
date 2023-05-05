#pragma once
#include "kson/chart_data.hpp"

namespace MusicGame::Camera
{
	inline constexpr double ScaledCamZoomValue(double zoom)
	{
		// 基準値である+100で画面に近過ぎるのを回避するため、zoomは正の値の場合には緩やかに推移させる
		if (zoom > 0.0)
		{
			return zoom / 450;
		}
		else
		{
			return zoom / 180;
		}
	}

	inline constexpr double JdglineScale(double zoom)
	{
		// 数式は見た目上の大きさがレーンとおおむね一致するように経験的に調整されたものであるため、各計算は特定の意味を持たない
		// HSP版: https://github.com/m4saka/kshootmania-v1-hsp/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L716-L723
		const double scaled = ScaledCamZoomValue(zoom);
		constexpr double scaled300 = -ScaledCamZoomValue(-300.0);
		double order = 5.0;
		if (zoom < 0.0)
		{
			order -= 3.0;
		}
		if (zoom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomValue(200.0));
		}
		return Max((Pow(Max(scaled + scaled300, 0.0) / scaled300, order) + 0.5) / 1.5, 1.0);
	}

	inline constexpr double JdgoverlayScale(double zoom)
	{
		// 数式は見た目上の大きさがレーンとおおむね一致するように経験的に調整されたものであるため、各計算は特定の意味を持たない
		// HSP版: https://github.com/m4saka/kshootmania-v1-hsp/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L707-L714
		const double scaled = ScaledCamZoomValue(zoom);
		constexpr double scaled300 = -ScaledCamZoomValue(-300.0);
		double order = 4.0;
		if (zoom < 0.0)
		{
			order -= 2.0;
		}
		if (zoom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomValue(200.0));
		}
		return (Pow(Max(scaled + scaled300, 0.0) / scaled300, order) + 0.4545 * Pow(1.1, Min((scaled - scaled300) / scaled300, 1.0))) / 1.5;
	}
}
