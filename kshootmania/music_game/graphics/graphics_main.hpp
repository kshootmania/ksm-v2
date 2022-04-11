#pragma once
#include "highway_3d_graphics.hpp"
#include "update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class GraphicsMain
	{
	private:
		BasicCamera3D m_camera;

		MSRenderTexture m_additive3dViewTexture;
		MSRenderTexture m_invMultiply3dViewTexture;

		Texture m_bgTexture;

		Highway3DGraphics m_highway3DGraphics;

	public:
		explicit GraphicsMain(const ksh::ChartData& chartData);

		void update(const UpdateInfo& updateInfo);

		void draw() const;
	};
}
