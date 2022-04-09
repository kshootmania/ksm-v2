#pragma once
#include "highway_3d_graphics.hpp"

namespace MusicGame
{
	class MusicGameGraphics
	{
	private:
		BasicCamera3D m_camera;

		MSRenderTexture m_3dViewTexture;

		Highway3DGraphics m_highway3DGraphics;

	public:
		MusicGameGraphics();

		void update(const CameraState& cameraState);

		void draw() const;
	};
}
