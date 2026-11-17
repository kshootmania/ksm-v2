#pragma once
#include "MusicGame/GameStatus.hpp"
#include "MusicGame/ViewStatus.hpp"
#include "MusicGame/Graphics/Highway/HighwayRenderTexture.hpp"

namespace MusicGame::Graphics
{
	class KeyBeamGraphics
	{
	private:
		const Texture m_beamTexture;

		// レーン消失の範囲で使用するマスク(画像が無い場合は空)
		const Texture m_beamWipeMaskTexture;

	public:
		KeyBeamGraphics();

		void draw(const GameStatus& gameStatus, const ViewStatus& viewStatus, const HighwayRenderTexture& target) const;
	};
}
