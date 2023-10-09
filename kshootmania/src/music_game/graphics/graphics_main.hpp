#pragma once
#include "highway/highway_3d_graphics.hpp"
#include "jdgline/jdgline_3d_graphics.hpp"
#include "jdgline/jdgoverlay_3d_graphics.hpp"
#include "jdgline/laser_cursor_3d_graphics.hpp"
#include "hud/song_info_panel.hpp"
#include "hud/score_panel.hpp"
#include "hud/gauge_panel.hpp"
#include "hud/combo_overlay.hpp"
#include "hud/frame_rate_monitor.hpp"
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"
#include "music_game/scroll/highway_scroll.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Graphics
{
	class GraphicsMain
	{
	private:
		BasicCamera3D m_camera;

		// TODO: separate into single class
		const Mesh m_bgBillboardMesh;
		Texture m_bgTexture;
		const Mat4x4 m_bgTransform;
		std::array<Array<RenderTexture>, 2> m_layerFrameTextures;
		const Mat4x4 m_layerTransform;

		Highway3DGraphics m_highway3DGraphics;

		Jdgline3DGraphics m_jdgline3DGraphics;
		Jdgoverlay3DGraphics m_jdgoverlay3DGraphics;
		LaserCursor3DGraphics m_laserCursor3DGraphics;

		SongInfoPanel m_songInfoPanel;
		ScorePanel m_scorePanel;
		GaugePanel m_gaugePanel;
		ComboOverlay m_comboOverlay;
		FrameRateMonitor m_frameRateMonitor;

		void drawBG(const ViewStatus& viewStatus) const;

		void drawLayer(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus) const;

	public:
		explicit GraphicsMain(const kson::ChartData& chartData, FilePathView parentPath, GaugeType gaugeType);

		void update(const ViewStatus& viewStatus);

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const;
	};
}
