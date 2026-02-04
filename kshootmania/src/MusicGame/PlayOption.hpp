#pragma once
#include "Scroll/HispeedSetting.hpp"

namespace MusicGame
{
	// テストプレイ用オプション
	struct TestPlayOption
	{
		Optional<GaugeType> gaugeType;
		Optional<int32> startMeasure;

		bool hasStartMeasure() const
		{
			return startMeasure.has_value() && *startMeasure > 0;
		}
	};

	struct PlayOption
	{
		GameMode gameMode = GameMode::kNormal;

		IsAutoPlayYN isAutoPlay = IsAutoPlayYN::No;

		GaugeType gaugeType = GaugeType::kNormalGauge;

		TurnMode turnMode = TurnMode::kNormal;

		double playbackSpeed = 1.0;

		JudgmentPlayMode btJudgmentPlayMode = JudgmentPlayMode::kOn;

		JudgmentPlayMode fxJudgmentPlayMode = JudgmentPlayMode::kOn;

		JudgmentPlayMode laserJudgmentPlayMode = JudgmentPlayMode::kOn;

		int32 globalOffsetMs = 0;

		int32 inputDelayMs = 0;

		int32 laserInputDelayMs = 0;

		int32 audioProcDelayMs = 0; // v1のsoundfx_delayにあたる

		int32 visualOffsetMs = 0;

		bool isAutoPlaySE = false; // FXチップ・直角の効果音のみオートプレイ

		NoteSkinType noteSkin = NoteSkinType::kDefault;

		FastSlowMode fastSlowMode = FastSlowMode::kHide;

		Array<HispeedType> availableHispeedTypes = { HispeedType::OMod };

		HispeedSetting hispeedSetting = HispeedSetting{ .type = HispeedType::OMod, .value = 500 };

		bool movieEnabled = true;

		bool showBG = true;

		bool showLayer = true;

		Optional<TestPlayOption> testPlayOption;

		// テストプレイで開始小節が指定されているか
		bool isTestPlayWithStartMeasure() const
		{
			return testPlayOption.has_value() && testPlayOption->hasStartMeasure();
		}

		// オートプレイを考慮したBT判定モードを取得
		JudgmentPlayMode effectiveBtJudgmentPlayMode() const
		{
			return isAutoPlay ? JudgmentPlayMode::kAuto : btJudgmentPlayMode;
		}

		// オートプレイを考慮したFX判定モードを取得
		JudgmentPlayMode effectiveFxJudgmentPlayMode() const
		{
			return isAutoPlay ? JudgmentPlayMode::kAuto : fxJudgmentPlayMode;
		}

		// オートプレイを考慮したLASER判定モードを取得
		JudgmentPlayMode effectiveLaserJudgmentPlayMode() const
		{
			return isAutoPlay ? JudgmentPlayMode::kAuto : laserJudgmentPlayMode;
		}

		// visualOffsetを考慮したglobalOffsetを取得
		int32 effectiveGlobalOffsetMs() const
		{
			return globalOffsetMs - visualOffsetMs;
		}

		// visualOffsetを考慮したinputDelayを取得(オートプレイ時は判定調整なし)
		int32 effectiveInputDelayMs() const
		{
			if (isAutoPlay)
			{
				return visualOffsetMs;
			}
			else
			{
				return inputDelayMs + visualOffsetMs;
			}
		}

		// オートプレイ時は判定調整なし
		int32 effectiveLaserInputDelayMs() const
		{
			return isAutoPlay ? 0 : laserInputDelayMs;
		}

		// visualOffsetを考慮したオーディオエフェクト遅延を取得
		int32 effectiveAudioProcDelayMs() const
		{
			return audioProcDelayMs + visualOffsetMs;
		}

		bool isAssist() const
		{
			return btJudgmentPlayMode != JudgmentPlayMode::kOn ||
				fxJudgmentPlayMode != JudgmentPlayMode::kOn ||
				laserJudgmentPlayMode != JudgmentPlayMode::kOn;
		}

		// スコアを保存すべきかどうか
		bool shouldSaveScore() const
		{
			return !isAutoPlay && !testPlayOption.has_value();
		}

		// playbackSpeed取得(ゼロ除算対策用に0の場合は1を返す)
		double nonZeroPlaybackSpeed() const
		{
			return playbackSpeed != 0.0 ? playbackSpeed : 1.0;
		}
	};
}
