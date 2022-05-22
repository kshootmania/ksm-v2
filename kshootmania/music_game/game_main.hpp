#pragma once
#include "music_game/judgment/button_lane_judgment.hpp"
#include "music_game/graphics/graphics_main.hpp"
#include "music_game/audio/bgm.hpp"
#include "music_game/audio/assist_tick.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame
{
	struct GameCreateInfo
	{
		FilePath chartFilePath;

		bool enableAssistTick = false;
	};

	class GameMain
	{
	private:
		// Chart
		const kson::ChartData m_chartData;
		const kson::TimingCache m_timingCache;

		// Judgment
		std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanes> m_btLaneJudgments;
		std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanes> m_fxLaneJudgments;
		const int32 m_scoreValueMax;

		// Audio
		Audio::BGM m_bgm;
		Audio::AssistTick m_assistTick;

		// Graphics
		Graphics::UpdateInfo m_graphicsUpdateInfo;
		Graphics::GraphicsMain m_musicGameGraphics;

		void registerAudioEffects();

	public:
		explicit GameMain(const GameCreateInfo& gameCreateInfo);

		void update();

		void draw() const;
	};
}
