#pragma once
#include "music_game/graphics/update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class FXNoteGraphics
	{
	private:
		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

	public:
		FXNoteGraphics();

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& subtractiveTarget) const;
	};
}
