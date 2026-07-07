#pragma once
#include "Course/CoursePlayState.hpp"
#include "GameStatus.hpp"
#include "PlayOption.hpp"
#include "PlayResult.hpp"
#include "Judgment/JudgmentMain.hpp"
#include "Camera/HighwayTilt.hpp"
#include "Scroll/HispeedSetting.hpp"
#include "Scroll/HighwayScroll.hpp"
#include "Audio/BGM.hpp"
#include "Audio/AssistTick.hpp"
#include "Audio/LaserSlamSE.hpp"
#include "Audio/FXChipSE.hpp"
#include "Audio/AudioEffectMain.hpp"
#include "UI/HispeedSettingMenu.hpp"
#include "Graphics/GraphicsMain.hpp"
#include "kson/Util/TimingUtils.hpp"
#include "Ini/FolderConfIni.hpp"

namespace MusicGame
{
	// S-RANDOMで元のFXレーン基準でエフェクト・効果音を鳴らすために使用するノーツ毎の対応関係
	struct SRandomFXAudioMapping
	{
		// turn適用前のFXノーツ(音声エフェクト用の押下中ロングノーツ特定用)
		kson::FXLane<kson::Interval> fxLanesForAudio;

		// 元のFXレーンにおける各ノーツの配置先レーン番号(0〜5)
		std::array<kson::ByPulse<std::size_t>, kson::kNumFXLanesSZ> fxNoteDestinations;
	};

	struct GameCreateInfo
	{
		FilePath chartFilePath;

		PlayOption playOption;

		AssistTickMode assistTickMode = AssistTickMode::kOff;

		Optional<CourseContinuation> courseContinuation = none;

		FolderConfIni folderConfIni;
	};

	class GameMain
	{
	public:
		using StartFadeOutYN = YesNo<struct StartFadeOutYN_tag>;

	private:
		FilePath m_chartFilePath;

		FilePath m_parentPath;

		// 初回更新かどうか
		// TODO: 消したい
		bool m_isFirstUpdate = true;

		// 譜面情報
		std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ> m_laserCurvedPulses;

		// turnオプション適用による配置結果
		NoteArrangement m_noteArrangement;

		// S-RANDOM時に音声を元のレーンで処理するための対応関係データ
		Optional<SRandomFXAudioMapping> m_sRandomFXAudioMapping;

		const kson::ChartData m_chartData;
		const kson::TimingCache m_timingCache;

		// プレイオプション
		const PlayOption m_playOption;

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
		Audio::FXChipSE m_fxChipSE;
		ksmaudio::Sample m_hardFailedSound;

		// 音声エフェクト
		Audio::AudioEffectMain m_audioEffectMain;

		// UI
		HispeedSettingMenu m_hispeedSettingMenu; // ハイスピード設定メニュー

		// グラフィックス
		Graphics::GraphicsMain m_graphicsMain;

		// 状態
		GameStatus m_gameStatus;
		ViewStatus m_viewStatus;
		bool m_isFinishedPrev = false;

		// Backボタンによる途中終了かどうか(未判定ノーツが残った状態でlockForExitが呼ばれた場合にtrue)
		bool m_isAborted = false;

		// 再生制御
		bool m_isPaused = false;
		Stopwatch m_fastForwardStopwatch;
		Stopwatch m_syncAdjustStopwatch;

		void updateStatus();

		void updateHighwayScroll();

		void processPlaybackControl();

		/// @brief S-RANDOM時に元のFXレーン基準でロングFXノーツの押下状態を取得
		[[nodiscard]]
		Optional<bool> sRandomLongFXPressed(std::size_t fxLaneIdx) const;

		/// @brief S-RANDOM時に元のFXレーン基準でチップノーツの判定情報を取得
		[[nodiscard]]
		std::array<Audio::FXChipJudgedStatus, kson::kNumFXLanesSZ> sRandomFXChipJudgedStatuses() const;

	public:
		explicit GameMain(const GameCreateInfo& createInfo);

		void start();

		StartFadeOutYN update();

		void draw() const;

		/// @brief プレイ終了のために判定処理をロックし、残りの未判定ノーツをERROR判定にする
		void lockForExit();

		void terminate();

		FilePathView chartFilePath() const;

		const kson::ChartData& chartData() const;

		PlayResult playResult() const;

		/// @brief タイミング調整オフセット
		[[nodiscard]]
		int32 timingAdjustOffsetMs() const;

		void startBGMFadeOut(Duration duration);
	};
}
