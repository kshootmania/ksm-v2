#pragma once
#include "highway_3d_graphics.hpp"
#include "update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class GraphicsMain
	{
	private:
		BasicCamera3D m_camera;

		MSRenderTexture m_3dViewTexture;

		Highway3DGraphics m_highway3DGraphics;

	public:
		GraphicsMain();

		void update(const UpdateInfo& updateInfo);

		void draw() const;
	};
}
