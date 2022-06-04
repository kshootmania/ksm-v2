#pragma once
#include "music_game/graphics/graphics_update_info.hpp"

namespace MusicGame::Graphics
{
	class Jdgline3DGraphics
	{
	private:
		const Texture m_jdglineTexture;

		const Mesh m_mesh;

	public:
		Jdgline3DGraphics();

		void draw3D(double tiltRadians) const;
	};
}
