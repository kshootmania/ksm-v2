#pragma once
#include "highway/highway_3d_graphics.hpp"
#include "highway/highway_tilt.hpp"
#include "jdgline/jdgline_3d_graphics.hpp"
#include "hud/song_info_panel.hpp"
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

		const Mesh m_billboardMesh;

		MSRenderTexture m_3dViewTexture;

		// TODO: separate into single class
		Texture m_bgTexture;
		const Mat4x4 m_bgTransform;
		std::array<Array<RenderTexture>, 2> m_layerFrameTextures;
		const Mat4x4 m_layerTransform;

		HighwayTilt m_highwayTilt;
		Highway3DGraphics m_highway3DGraphics;

		Jdgline3DGraphics m_jdgline3DGraphics;

		SongInfoPanel m_songInfoPanel;
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
