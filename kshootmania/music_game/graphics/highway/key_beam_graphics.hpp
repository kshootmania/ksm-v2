#pragma once
#include "music_game/graphics/graphics_update_info.hpp"

namespace MusicGame::Graphics
{
	class KeyBeamGraphics
	{
	private:
		const Texture m_beamTexture;

	public:
		KeyBeamGraphics();

		void draw(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget) const;
	};
}
