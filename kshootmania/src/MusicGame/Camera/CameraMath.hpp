#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"

namespace MusicGame::Camera
{
	inline constexpr double ScaledCamZoomBottomValue(double zoomBottom)
	{
		// 基準値である+100で画面に近過ぎるのを回避するため、zoomBottomは正の値の場合には緩やかに推移させる
		if (zoomBottom > 0.0)
		{
			return zoomBottom / 450;
		}
		else
		{
			return zoomBottom / 180;
		}
	}

	inline constexpr double ScaledLegacyZoomTop(double zoomTop)
	{
		// KSHバージョン167未満でのzoom_top値の変換
		// HSP版: https://github.com/kshootmania/ksm-v1/blob/ea05374a3ece796612b29d927cb3c6f5aabb266e/src/scene/play/play_draw_frame.hsp#L700-L705
		double scaled = zoomTop / 1.5 / 50.0;
		if (scaled <= 0.0)
		{
			scaled /= 2.5;
		}
		return scaled;
	}

	inline Mat4x4 CamShiftXMatrix(double shiftX)
	{
		return Mat4x4::Translate(Vec3::Right(shiftX));
	}

	inline constexpr double JdglineScale(double zoomBottom)
	{
		// 数式は見た目上の大きさがレーンとおおむね一致するように経験的に調整されたものであるため、各計算は特定の意味を持たない
		// HSP版: https://github.com/kshootmania/ksm-v1/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L716-L723
		const double scaled = ScaledCamZoomBottomValue(zoomBottom);
		constexpr double scaled300 = -ScaledCamZoomBottomValue(-300.0);
		double order = 5.0;
		if (zoomBottom < 0.0)
		{
			order -= 3.0;
		}
		if (zoomBottom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomBottomValue(200.0));
		}
		return Max((Pow(Max(scaled + scaled300, 0.0) / scaled300, order) + 0.5) / 1.5, 1.0);
	}

	inline constexpr double LegacyJdglineScale(double zoomBottom)
	{
		// KSHバージョン167未満での判定ラインの大きさ計算
		// HSP版: https://github.com/kshootmania/ksm-v1/blob/ea05374a3ece796612b29d927cb3c6f5aabb266e/src/scene/play/play_draw_frame.hsp#L730-L737
		const double scaled = ScaledCamZoomBottomValue(zoomBottom);
		constexpr double scaled300 = -ScaledCamZoomBottomValue(-300.0);
		double order = 5.0;
		if (zoomBottom < 0.0)
		{
			order -= 3.0;
		}
		if (zoomBottom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomBottomValue(200.0));
		}
		return (Pow((scaled + scaled300) / scaled300, order) + 0.5) / 1.5;
	}

	inline constexpr double JdgoverlayScale(double zoomBottom)
	{
		// 数式は見た目上の大きさがレーンとおおむね一致するように経験的に調整されたものであるため、各計算は特定の意味を持たない
		// HSP版: https://github.com/kshootmania/ksm-v1/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L707-L714
		const double scaled = ScaledCamZoomBottomValue(zoomBottom);
		constexpr double scaled300 = -ScaledCamZoomBottomValue(-300.0);
		double order = 4.0;
		if (zoomBottom < 0.0)
		{
			order -= 2.0;
		}
		if (zoomBottom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomBottomValue(200.0));
		}
		return (Pow(Max(scaled + scaled300, 0.0) / scaled300, order) + 0.4545 * Pow(1.1, Min((scaled - scaled300) / scaled300, 1.0))) / 1.5;
	}

	inline constexpr double LegacyJdgoverlayScale(double zoomBottom)
	{
		// KSHバージョン167未満での判定アニメーションの大きさ計算
		// HSP版: https://github.com/kshootmania/ksm-v1/blob/ea05374a3ece796612b29d927cb3c6f5aabb266e/src/scene/play/play_draw_frame.hsp#L721-L728
		const double scaled = ScaledCamZoomBottomValue(zoomBottom);
		constexpr double scaled300 = -ScaledCamZoomBottomValue(-300.0);
		double order = 4.0;
		if (zoomBottom < 0.0)
		{
			order -= 2.0;
		}
		if (zoomBottom > 200.0)
		{
			order += 10.0 * (scaled - ScaledCamZoomBottomValue(200.0));
		}
		return (Pow((scaled + scaled300) / scaled300, order) + 0.5) / 1.5;
	}

	inline constexpr double CenterSplitShiftX(double centerSplit, double btLaneDiffX = Graphics::kBTLanePositionDiff.x)
	{
		constexpr double kOneLaneCenterSplit = 100.0;
		return btLaneDiffX * centerSplit / kOneLaneCenterSplit / 2;
	}

	inline constexpr double HighwayWipeRate(double highwayWipe)
	{
		constexpr double kHighwayWipeAbsMax = 100.0;
		return Min(Abs(highwayWipe), kHighwayWipeAbsMax) / kHighwayWipeAbsMax;
	}
}
