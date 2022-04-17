#pragma once
#include "music_game/graphics/update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class LaserNoteGraphics
	{
	private:
		const Texture m_laserNoteTexture;
		const Texture m_laserNoteMaskTexture;
		const TiledTexture m_laserNoteLeftStartTexture;
		const TiledTexture m_laserNoteRightStartTexture;

	public:
		LaserNoteGraphics();

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
