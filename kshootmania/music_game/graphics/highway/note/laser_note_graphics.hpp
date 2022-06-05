#pragma once
#include "music_game/game_status.hpp"

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

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
