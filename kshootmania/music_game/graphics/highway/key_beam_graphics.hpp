#pragma once
#include "music_game/graphics/update_info.hpp"

namespace MusicGame::Graphics
{
	class KeyBeamGraphics
	{
	private:
		const Texture m_beamTexture;

	public:
		KeyBeamGraphics();

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget) const;
	};
}
