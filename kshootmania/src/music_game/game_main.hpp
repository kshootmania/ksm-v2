#pragma once
#include "game_status.hpp"
#include "judgment/judgment_main.hpp"
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
		FilePath m_parentPath;

		// 初回更新かどうか
		// TODO: 消したい
		bool m_isFirstUpdate = true;

		// 譜面情報
		const kson::ChartData m_chartData;
		const kson::TimingCache m_timingCache;

		// 判定
		Judgment::JudgmentMain m_judgmentMain;

		// 視点制御
		Camera::HighwayTilt m_highwayTilt; // 傾き
		Camera::CamSystem m_camSystem; // 視点変更

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

		// 状態
		GameStatus m_gameStatus;
		ViewStatus m_viewStatus;

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
