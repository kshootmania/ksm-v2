#include "jdgoverlay_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "kson/common/common.hpp"
#include "laser_cursor_3d_graphics.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kTextureFilename = U"laser_cur.png";
		constexpr Size kSourceSize = { 64, 64 };

		constexpr Float3 kPlaneCenter = { 0.0f, 3.0f, -kHighwayPlaneSize.y / 2 - 1.0f };
		constexpr Float2 kPlaneSize = { 4.5f, 4.5f };

		Mat4x4 CursorTransform(bool wide, double cursorX)
		{
			const int32 xScale = wide ? kLaserXScaleWide : kLaserXScaleNormal;
			return Mat4x4::Translate((cursorX - 0.5) * 282 / 8 * xScale, 0.0, 0.0);
		}
	}

	LaserCursor3DGraphics::LaserCursor3DGraphics()
		: m_texture(kTextureFilename,
			{
				.row = kson::kNumLaserLanes,
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kSourceSize,
			})
		, m_mesh(MeshData::Grid(kPlaneCenter, kPlaneSize, 2, 2))
	{
	}

	void LaserCursor3DGraphics::draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus) const
	{
		// LASERカーソルを3D空間上に描画
		const ScopedRenderStates3D blendState(kEnableAlphaBlend);
		const Transformer3D baseTransform(JudgmentPlaneTransformMatrix(viewStatus.tiltRadians, kPlaneCenter));
		for (int32 i = 0; i < kson::kNumLaserLanes; ++i)
		{
			const auto& laneStatus = gameStatus.laserLaneStatus[i];
			if (!laneStatus.cursorX.has_value())
			{
				// カーソルが出ていない
				continue;
			}

			// カーソルを描画
			const double cursorX = laneStatus.cursorX.value();
			const Transformer3D cursorTransform(CursorTransform(laneStatus.cursorWide, cursorX));
			m_mesh.draw(m_texture(i));
		}
	}
}
