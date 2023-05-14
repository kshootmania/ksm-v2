#pragma once
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"
#include "music_game/graphics/highway/highway_render_texture.hpp"

namespace MusicGame::Graphics
{
	class KeyBeamGraphics
	{
	private:
		const Texture m_beamTexture;

	public:
		KeyBeamGraphics();

		void draw(const GameStatus& gameStatus, const ViewStatus& viewStatus, const HighwayRenderTexture& target) const;
	};
}
