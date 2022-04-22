#pragma once
#include "highway/highway_3d_graphics.hpp"
#include "highway/highway_tilt.hpp"
#include "jdgline/jdgline_3d_graphics.hpp"
#include "hud/score_panel.hpp"
#include "hud/gauge_panel.hpp"
#include "hud/frame_rate_monitor.hpp"
#include "update_info.hpp"
#include "ksh/util/timing_utils.hpp"

namespace MusicGame::Graphics
{
	class GraphicsMain
	{
	private:
		BasicCamera3D m_camera;

		Texture m_bgTexture;
		TiledTexture m_layerTexture;

		MSRenderTexture m_highwayAdditiveLayer;
		MSRenderTexture m_highwayInvMultiplyLayer;
		HighwayTilt m_highwayTilt;
		Highway3DGraphics m_highway3DGraphics;

		MSRenderTexture m_jdglineLayer;
		Jdgline3DGraphics m_jdgline3DGraphics;

		ScorePanel m_scorePanel;
		GaugePanel m_gaugePanel;
		FrameRateMonitor m_frameRateMonitor;

		ksh::Pulse m_initialPulse;

		UpdateInfo m_updateInfo;

	public:
		explicit GraphicsMain(const ksh::ChartData& chartData, const ksh::TimingCache& timingCache);

		void update(const UpdateInfo& updateInfo);

		void draw() const;
	};
}
