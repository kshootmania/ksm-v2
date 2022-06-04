#pragma once
#include "highway/highway_3d_graphics.hpp"
#include "highway/highway_tilt.hpp"
#include "jdgline/jdgline_3d_graphics.hpp"
#include "jdgoverlay/jdgoverlay_3d_graphics.hpp"
#include "hud/song_info_panel.hpp"
#include "hud/score_panel.hpp"
#include "hud/gauge_panel.hpp"
#include "hud/frame_rate_monitor.hpp"
#include "graphics_update_info.hpp"
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

		HighwayTilt m_highwayTilt;
		Highway3DGraphics m_highway3DGraphics;

		Jdgline3DGraphics m_jdgline3DGraphics;

		Jdgoverlay3DGraphics m_jdgoverlay3DGraphics;

		SongInfoPanel m_songInfoPanel;
		ScorePanel m_scorePanel;
		GaugePanel m_gaugePanel;
		FrameRateMonitor m_frameRateMonitor;

		kson::Pulse m_initialPulse;

		GraphicsUpdateInfo m_updateInfo;

		void drawBG() const;

		void drawLayer() const;

	public:
		explicit GraphicsMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(const GraphicsUpdateInfo& updateInfo);

		void draw() const;
	};
}
