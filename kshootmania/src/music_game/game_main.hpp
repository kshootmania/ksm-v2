#pragma once
#include "game_status.hpp"
#include "judgment/button_lane_judgment.hpp"
#include "judgment/laser_lane_judgment.hpp"
#include "camera/highway_tilt.hpp"
#include "scroll/hispeed_setting.hpp"
#include "scroll/highway_scroll.hpp"
#include "audio/bgm.hpp"
#include "audio/assist_tick.hpp"
#include "audio/laser_slam_se.hpp"
#include "audio/audio_effect_main.hpp"
#include "ui/hispeed_setting_menu.hpp"
#include "graphics/graphics_main.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame
{
	struct GameCreateInfo
	{
		FilePath chartFilePath;

		bool assistTickEnabled = false;
	};

	class GameMain
	{
	private:
		GameStatus m_gameStatus;
		ViewStatus m_viewStatus;

		FilePath m_parentPath;

		// 譜面情報
		const kson::ChartData m_chartData;
		const kson::TimingCache m_timingCache;

		// 判定
		std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanesSZ> m_btLaneJudgments;
		std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanesSZ> m_fxLaneJudgments;
		std::array<Judgment::LaserLaneJudgment, kson::kNumLaserLanesSZ> m_laserLaneJudgments;
		const int32 m_scoreFactorMax;

		// 視点制御
		Camera::HighwayTilt m_highwayTilt; // 傾き

		// スクロール(ハイスピード・scroll_speedの処理)
		Scroll::HighwayScroll m_highwayScroll;

		// 音声
		Audio::BGM m_bgm;
		Audio::AssistTick m_assistTick;
		Audio::LaserSlamSE m_laserSlamSE;

		// 音声エフェクト
		Audio::AudioEffectMain m_audioEffectMain;

		// UI
		HispeedSettingMenu m_hispeedSettingMenu; // ハイスピード設定メニュー

		// グラフィックス
		Graphics::GraphicsMain m_graphicsMain;

		void updateGameStatus();

		void updateViewStatus();

		void updateHighwayScroll();

	public:
		explicit GameMain(const GameCreateInfo& createInfo);

		void start();

		void update();

		void draw() const;

		void terminate();
	};
}
