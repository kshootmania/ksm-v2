#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class KeyBeamGraphics
	{
	private:
		const Texture m_beamTexture;

	public:
		KeyBeamGraphics();

		void draw(const GameStatus& gameStatus, const RenderTexture& additiveTarget) const;
	};
}
