#pragma once
#include "music_game/graphics/update_info.hpp"

namespace MusicGame::Graphics
{
	class ButtonNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;

		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

	public:
		ButtonNoteGraphics();

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
