#pragma once
#include "music_game/graphics/graphics_main.hpp"
#include "music_game/audio/bgm.hpp"
#include "ksh/util/timing_utils.hpp"

namespace MusicGame
{
	struct GameCreateInfo
	{
		FilePath chartFilePath;
	};

	class GameMain
	{
	private:
		const ksh::ChartData m_chartData;
		const ksh::TimingCache m_timingCache;

		// Audio
		Audio::BGM m_bgm;

		// Graphics
		Graphics::UpdateInfo m_graphicsUpdateInfo;
		Graphics::GraphicsMain m_musicGameGraphics;

	public:
		GameMain(const GameCreateInfo& gameCreateInfo);

		void update();

		void draw() const;
	};
}
