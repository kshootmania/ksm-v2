#include "audio_effect_main.hpp"
#include "audio_effect_utils.hpp"
#include "ksmaudio/audio_effect/audio_effect_bus.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

		constexpr std::string_view kDefaultLaserAudioEffectName = "peaking_filter";

		void RegisterAudioEffects(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			using AudioEffectUtils::PrecalculateUpdateTriggerTiming;

			const std::int64_t totalMeasures =
				kson::SecToMeasureIdx(bgm.durationSec(), chartData.beat, timingCache)
				+ 1/* 最後の小節の分を足す */
				+ 1/* インデックスを要素数にするために1を足す */;

			// FX
			for (const auto& [name, def] : chartData.audio.audioEffect.fx.def)
			{
				const auto& paramChangeDict = chartData.audio.audioEffect.fx.paramChange;
				const std::set<float> updateTriggerTiming =
					paramChangeDict.contains(name)
					? PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, chartData, timingCache)
					: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);

				bgm.emplaceAudioEffectFX(name, def, updateTriggerTiming);
			}

			// Laser
			for (const auto& [name, def] : chartData.audio.audioEffect.laser.def)
			{
				const auto& paramChangeDict = chartData.audio.audioEffect.laser.paramChange;
				const std::set<float> updateTriggerTiming =
					paramChangeDict.contains(name)
					? PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, chartData, timingCache)
					: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);

				bgm.emplaceAudioEffectLaser(name, def, updateTriggerTiming);
			}

			// テスト用
			// TODO: Remove this code
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Retrigger };
				const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
				bgm.emplaceAudioEffectFX("retrigger", def, updateTriggerTiming);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Gate };
				const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
				bgm.emplaceAudioEffectFX("gate", def, updateTriggerTiming);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Flanger };
				bgm.emplaceAudioEffectFX("flanger", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Bitcrusher };
				bgm.emplaceAudioEffectFX("bitcrusher", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Wobble };
				const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
				bgm.emplaceAudioEffectFX("wobble", def, updateTriggerTiming);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Tapestop };
				bgm.emplaceAudioEffectFX("tapestop", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Echo };
				const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
				bgm.emplaceAudioEffectFX("echo", def, updateTriggerTiming);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::PeakingFilter };
				bgm.emplaceAudioEffectLaser("peaking_filter", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::HighPassFilter };
				bgm.emplaceAudioEffectLaser("high_pass_filter", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::LowPassFilter };
				bgm.emplaceAudioEffectLaser("low_pass_filter", def);
			}
			{
				const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Bitcrusher };
				bgm.emplaceAudioEffectLaser("bitcrusher", def);
			}
		}

		kson::FXLane<AudioEffectInvocation> CreateLongFXNoteAudioEffectInvocations(BGM& bgm, const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			const auto& audioEffectBus = bgm.audioEffectBusFX();
			kson::FXLane<AudioEffectInvocation> convertedLongEvent;
			for (const auto& [audioEffectName, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
				{
					for (const auto& [y, dict] : lanes[i])
					{
						if (!audioEffectBus.audioEffectContainsName(audioEffectName))
						{
							// FXノーツに定義されていないエフェクト名が指定されている場合は無視
							continue;
						}

						convertedLongEvent[i].insert_or_assign(y, AudioEffectInvocation{
							.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(audioEffectName),
							.overrideParams = ksmaudio::AudioEffect::StrDictToParamValueSetDict(dict),
						});
					}
				}
			}
			return convertedLongEvent;
		}

		kson::ByPulse<Optional<AudioEffectInvocation>> CreateLaserPulseAudioEffectInvocations(BGM& bgm, const kson::ChartData& chartData)
		{
			const auto& pulseEvent = chartData.audio.audioEffect.laser.pulseEvent;
			const auto& audioEffectBus = bgm.audioEffectBusLaser();
			kson::ByPulse< Optional<AudioEffectInvocation>> convertedPulseEvent;
			for (const auto& [audioEffectName, pulses] : pulseEvent)
			{
				for (const auto& y : pulses)
				{
					if (audioEffectBus.audioEffectContainsName(audioEffectName))
					{
						convertedPulseEvent.insert_or_assign(y, AudioEffectInvocation{
							.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(audioEffectName),
						});
					}
					else
					{
						// LASERに定義されていないエフェクト名が指定されている場合はエフェクトなしにする
						convertedPulseEvent.insert_or_assign(y, none);
					}
				}
			}

			// LASERエフェクト呼び出しがない、または、初回のLASERエフェクト呼び出しがゼロ地点より後ろの場合は、ゼロ地点にデフォルト値を挿入
			if (convertedPulseEvent.empty() || convertedPulseEvent.begin()->first > kson::Pulse{ 0 })
			{
				const std::string defaultAudioEffectName{ kDefaultLaserAudioEffectName };
				if (audioEffectBus.audioEffectContainsName(defaultAudioEffectName))
				{
					convertedPulseEvent.insert_or_assign(kson::Pulse{ 0 }, AudioEffectInvocation{
						.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(defaultAudioEffectName),
					});
				}
				else
				{
					// LASERにデフォルト値の音声エフェクトが定義されていない場合はエフェクトなしにする
					convertedPulseEvent.insert_or_assign(kson::Pulse{ 0 }, none);
				}
			}

			return convertedPulseEvent;
		}

		Optional<std::pair<kson::Pulse, kson::Interval>> CurrentLongNoteByTime(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto currentNoteItr = kson::ValueItrAt(lane, currentPulse);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, currentNote] = *currentNoteItr;
				if (y <= currentPulse && currentPulse < y + currentNote.length)
				{
					return *currentNoteItr;
				}
			}
			return none;
		}
	}

	void AudioEffectMain::updateActiveAudioEffectDictFX(
		const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio)
	{
		m_activeAudioEffectDictFX.clear();

		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			// 2レーンのうち直近ロングFXノーツが押され始めた方のレーンを先に処理する
			// (2レーンで互いに異なるパラメータ値を持つ同じ音声エフェクトが指定される場合があり、直近の方を優先する必要があるため。
			//  例: "wave_length=1/8"のRetriggerと"wave_length=1/16"のRetriggerが同時に配置されている場合、"wave_length"は後に押した方で指定された値にする必要がある)
			// なお, ActiveAudioEffectDict::emplace()では同じキーの2回目の挿入は無視される
			static_assert(kson::kNumFXLanesSZ == 2U); // 下のコードはkNumFXLanesSZが2である前提である
			const std::size_t laneIdx = (i == 0) ? m_lastPressedLongFXNoteLaneIdx : (1U - m_lastPressedLongFXNoteLaneIdx);
			assert(laneIdx < kson::kNumFXLanesSZ);

			if (!currentLongNoteOfLanes[laneIdx].has_value())
			{
				continue;
			}

			const auto& [longNoteY, longNote] = *currentLongNoteOfLanes[laneIdx];
			const auto itr = kson::ValueItrAt(m_longFXNoteInvocations[laneIdx], currentPulseForAudio);
			if (itr == m_longFXNoteInvocations[laneIdx].end())
			{
				continue;
			}

			// 1本のロングFXノーツの途中で他の種類の音声エフェクトに変更される場合がある。
			// そのため、イベントが現在のロングノーツに属するかを調べるには、ロングノーツの開始点との一致判定ではなく、ロングノーツの範囲(開始～終了)の中にあるかを調べる必要がある。
			const auto& [longEventY, audioEffectInvocation] = *itr;
			if (longEventY > currentPulseForAudio || longEventY < longNoteY || longNoteY + longNote.length <= longEventY)
			{
				continue;
			}
			m_activeAudioEffectDictFX.emplace(audioEffectInvocation.audioEffectIdx, ksmaudio::AudioEffect::ActiveAudioEffectInvocation{
				.pOverrideParams = &audioEffectInvocation.overrideParams,
				.laneIdx = laneIdx,
			});
		}
	}

	std::optional<std::size_t> AudioEffectMain::getActiveLaserAudioEffectIdx(kson::Pulse currentPulseForAudio)
	{
		assert(!m_laserPulseInvocations.empty()); // ゼロ地点にデフォルト値が挿入されるのでここでは空ではないはず

		const auto& [y, audioEffectInvocation] = *kson::ValueItrAt(m_laserPulseInvocations, currentPulseForAudio);
		if (audioEffectInvocation.has_value())
		{
			return audioEffectInvocation->audioEffectIdx;
		}
		else
		{
			return std::nullopt;
		}
	}

	AudioEffectMain::AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_longFXNoteInvocations((RegisterAudioEffects(bgm, chartData, timingCache), CreateLongFXNoteAudioEffectInvocations(bgm, chartData))) // 先に登録しておく必要があるので、分かりにくいがカンマ演算子を使用している(TODO: もうちょっとどうにかする)
		, m_laserPulseInvocations(CreateLaserPulseAudioEffectInvocations(bgm, chartData))
	{
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus)
	{
		const double currentTimeSec = bgm.posSec();
		const double currentTimeSecForAudio = currentTimeSec + bgm.latencySec(); // Note: In BASS v2.4.13 and later, for unknown reasons, the effects are out of sync even after adding this latency.
		const kson::Pulse currentPulseForAudio = kson::SecToPulse(currentTimeSecForAudio, chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TempoAt(currentPulseForAudio, chartData.beat);

		// ロングFXノーツの音声エフェクト
		bool bypassFX = true;
		std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ> currentLongNoteOfLanes;
		for (std::size_t i = 0; i < kson::kNumFXLanesSZ; ++i)
		{
			const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForAudio);

			// 音声エフェクトはバッファサイズによる遅延を回避するため、バッファサイズ分だけ早めに適用し始める必要がある。
			// これはプレイヤーのキー入力に関係なく実施する必要があるため、下記の「～.value_or(true)」で、longFXPressed[i]がnoneである(つまり判定中のロングFXノーツがまだ存在しない)場合でも、
			// あたかもロングFXが押されている(trueである)かのように扱っている。
			// 
			// それに加えて、キー入力がやや遅れた場合でも音声エフェクトが途切れないように、(バッファサイズとは別に)最初の30ms分はプレイヤーのキー入力に関係なく音声エフェクトを有効にしている。
			// 
			// HSP版での対応箇所: https://github.com/m4saka/kshootmania-v1-hsp/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_audio_effects.hsp#L488
			if (currentLongNoteByTime.has_value()
				&& (inputStatus.longFXPressed[i].value_or(true)
					|| (currentTimeSec - kson::PulseToSec(currentLongNoteByTime->first, chartData.beat, timingCache)) < kLongFXNoteAudioEffectAutoPlaySec))
			{
				currentLongNoteOfLanes[i] = currentLongNoteByTime;
				bypassFX = false;
				if (!m_longFXPressedPrev[i])
				{
					m_lastPressedLongFXNoteLaneIdx = i;
					m_longFXPressedPrev[i] = true;
				}
			}
			else
			{
				currentLongNoteOfLanes[i] = none;
				m_longFXPressedPrev[i] = false;
			}
		}
		updateActiveAudioEffectDictFX(currentLongNoteOfLanes, currentPulseForAudio);
		bgm.updateAudioEffectFX(
			bypassFX,
			{
				.bpm = static_cast<float>(currentBPMForAudio),
				.sec = static_cast<float>(currentTimeSecForAudio),
			},
			m_activeAudioEffectDictFX);

		// LASERノーツの音声エフェクト
		// TODO: 直角時のクリッピングを抑える
		bool bypassLaser = true;
		float laserValue = 0.0f;
		for (std::size_t i = 0; i < kson::kNumLaserLanesSZ; ++i)
		{
			if (!inputStatus.laserIsOnOrNone[i])
			{
				continue;
			}

			const auto& laneLaserValue = kson::GraphSectionValueAt(chartData.note.laser[i], currentPulseForAudio);
			if (laneLaserValue.has_value())
			{
				const float fLaneLaserValue = static_cast<float>(laneLaserValue.value());

				// 表示上のX座標の値(0.0:左、1.0:右)を、音声エフェクトで使うためのレーンに依存しない値(0.0～1.0)に変換
				static_assert(kson::kNumLaserLanesSZ == 2U); // 以下の処理はLASERのレーン数が2であることを前提にしている
				const float v = (i == 0U) ? fLaneLaserValue : (1.0f - fLaneLaserValue); // 右レーンの値を反転

				// 音声エフェクトにはLASERの左右レーンのうち値が大きい方を採用する
				if (laserValue < v)
				{
					laserValue = v;
				}
				bypassLaser = false;
			}
		}
		bgm.updateAudioEffectLaser(
			bypassLaser,
			{
				.v = laserValue,
				.bpm = static_cast<float>(currentBPMForAudio),
				.sec = static_cast<float>(currentTimeSecForAudio),
			},
			getActiveLaserAudioEffectIdx(currentPulseForAudio));
	}
}
