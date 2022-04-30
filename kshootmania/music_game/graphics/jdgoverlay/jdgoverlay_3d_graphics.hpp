#pragma once
#include "music_game/graphics/update_info.hpp"

namespace MusicGame::Graphics
{
	class Jdgoverlay3DGraphics
	{
	private:
		const RenderTexture m_renderTexture;
		const RenderTexture m_renderTexture2;

		const TiledTexture m_chipCriticalTexture;
		const TiledTexture m_chipNearTexture;
		const TiledTexture m_chipErrorTexture;

		const Mesh m_mesh;

		const TiledTexture& chipTexture(Judgment::JudgmentResult type) const;

	public:
		Jdgoverlay3DGraphics();

		void draw(const UpdateInfo& updateInfo, double tiltRadians) const;
	};
}
