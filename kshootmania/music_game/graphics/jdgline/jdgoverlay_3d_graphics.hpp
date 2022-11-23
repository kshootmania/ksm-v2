#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class Jdgoverlay3DGraphics
	{
	private:
		const RenderTexture m_renderTexture;

		const TiledTexture m_chipCriticalTexture;
		const TiledTexture m_chipNearTexture;
		const TiledTexture m_chipErrorTexture;

		const TiledTexture m_longAnimTexture;

		const TiledTexture m_laserAnimTexture;

		const Mat4x4 m_transform;
		const Mesh m_mesh;

		const TiledTexture& chipAnimTexture(Judgment::JudgmentResult type) const;

		void drawChipAnimCommon(const GameStatus& gameStatus, bool isBT) const;

		void drawChipAnimBT(const GameStatus& gameStatus) const;

		void drawChipAnimFX(const GameStatus& gameStatus) const;

		void drawLongAnimCommon(const GameStatus& gameStatus, bool isBT) const;

		void drawLongAnimBT(const GameStatus& gameStatus) const;

		void drawLongAnimFX(const GameStatus& gameStatus) const;

		void drawLaserAnim(const GameStatus& gameStatus) const;

	public:
		explicit Jdgoverlay3DGraphics(const BasicCamera3D& camera);

		void draw2D(const GameStatus& gameStatus) const;

		void draw3D(double tiltRadians) const;
	};
}
