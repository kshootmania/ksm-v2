#pragma once
#include "music_game/graphics/update_info.hpp"

namespace MusicGame::Graphics
{
	class Jdgoverlay3DGraphics
	{
	private:
		const RenderTexture m_renderTexture;

		const TiledTexture m_chipCriticalTexture;
		const TiledTexture m_chipNearTexture;
		const TiledTexture m_chipErrorTexture;

		const Mesh m_mesh;

		const TiledTexture& chipTexture(Judgment::JudgmentResult type) const;

	public:
		Jdgoverlay3DGraphics();

		void draw2D(const UpdateInfo& updateInfo) const;

		void draw3D(double tiltRadians) const;
	};
}
