#pragma once
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"

namespace MusicGame::Graphics
{
	class LaserCursor3DGraphics
	{
	private:
		const TiledTexture m_texture;

		const Mesh m_mesh;

	public:
		LaserCursor3DGraphics();

		void draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus, const BasicCamera3D& camera) const;
	};
}
