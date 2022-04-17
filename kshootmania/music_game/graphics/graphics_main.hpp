#pragma once
#include "highway/highway_3d_graphics.hpp"
#include "update_info.hpp"
#include "ksh/util/timing_utils.hpp"

namespace MusicGame::Graphics
{
	class GraphicsMain
	{
	private:
		BasicCamera3D m_camera;

		MSRenderTexture m_additive3dViewTexture;
		MSRenderTexture m_invMultiply3dViewTexture;

		Texture m_bgTexture;
		TiledTexture m_layerTexture;

		ksh::Pulse m_initialPulse;

		UpdateInfo m_updateInfo;

		Highway3DGraphics m_highway3DGraphics;

	public:
		explicit GraphicsMain(const ksh::ChartData& chartData, const ksh::TimingCache& timingCache);

		void update(const UpdateInfo& updateInfo);

		void draw() const;
	};
}
