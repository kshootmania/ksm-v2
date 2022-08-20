#pragma once
#include "game_status.hpp"
#include "music_game/judgment/button_lane_judgment.hpp"
#include "music_game/graphics/graphics_main.hpp"
#include "music_game/audio/bgm.hpp"
#include "music_game/audio/assist_tick.hpp"
#include "music_game/audio/audio_effect_main.hpp"
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
		GameStatus m_gameStatus;

		FilePath m_parentPath;

		// 譜面情報
		const kson::ChartData m_chartData;
		const kson::TimingCache m_timingCache;

		// 判定
		std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanesSZ> m_btLaneJudgments;
		std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanesSZ> m_fxLaneJudgments;
		const int32 m_scoreFactorMax;

		// 音声
		Audio::BGM m_bgm;
		Audio::AssistTick m_assistTick;

		// 音声エフェクト
		Audio::AudioEffectMain m_audioEffectMain;

		// グラフィックス
		Graphics::GraphicsMain m_graphicsMain;

		void updateGameStatus();

	public:
		explicit GameMain(const GameCreateInfo& gameCreateInfo);

		void update();

		void draw() const;
	};
}
