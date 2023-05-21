#pragma once
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"

namespace MusicGame::Graphics
{
	class Jdgline3DGraphics
	{
	private:
		const Texture m_jdglineTexture;

		const Mesh m_mesh;

	public:
		Jdgline3DGraphics();

		void draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus) const;
	};
}
