#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class LaserCursor3DGraphics
	{
	private:
		const TiledTexture m_texture;

		const Mesh m_mesh;

	public:
		LaserCursor3DGraphics();

		void draw3D(double tiltRadians, const GameStatus& gameStatus) const;
	};
}
