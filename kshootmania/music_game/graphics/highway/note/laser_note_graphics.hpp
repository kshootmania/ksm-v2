#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class LaserNoteGraphics
	{
	private:
		const Texture m_laserNoteTexture;
		const Texture m_laserNoteMaskTexture;
		const std::array<TiledTexture, kson::kNumLaserLanesSZ> m_laserNoteStartTextures;

		void drawSection(int32 laneIdx, kson::RelPulse relPulse, const kson::LaserSection& laserSection, const RenderTexture& target, const Texture& laserNoteTexture, const TextureRegion& laserStartTexture) const;

	public:
		LaserNoteGraphics();

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
