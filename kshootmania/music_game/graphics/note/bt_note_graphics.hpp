#pragma once
#include "music_game/graphics/update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class BTNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;

	public:
		BTNoteGraphics();

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
