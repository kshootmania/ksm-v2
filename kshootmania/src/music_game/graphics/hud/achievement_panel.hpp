#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class AchievementPanel
	{
	private:
		const TiledTexture m_texture;

	public:
		AchievementPanel();

		void draw(const GameStatus& gameStatus) const;
	};
}
