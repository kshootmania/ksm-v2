#include "audio_effect_main.hpp"
#include "audio_effect_utils.hpp"
#include "ksmaudio/audio_effect/audio_effect_bus.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

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
		}

		kson::FXLane<AudioEffectInvocation> CreateLongFXNoteAudioEffectInvocations(BGM& bgm, const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			kson::FXLane<AudioEffectInvocation> convertedLongEvent;
			for (const auto& [audioEffectName, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
				{
					for (const auto& [y, dict] : lanes[i])
					{
						if (!bgm.audioEffectBusFX().audioEffectContainsName(audioEffectName))
						{
							// FXノーツに定義されていないエフェクト名が指定されている場合は無視
							continue;
						}

						convertedLongEvent[i].emplace(y, AudioEffectInvocation{
							.audioEffectIdx = bgm.audioEffectBusFX().audioEffectNameToIdx(audioEffectName),
							.overrideParams = ksmaudio::AudioEffect::StrDictToParamValueSetDict(dict),
						});
					}
				}
			}
			return convertedLongEvent;
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

	// TODO:
	// 現状"updateTrigger=off>on"がFX-L/Rに交互に降ってきた時に両方トリガできないので、戻り値の形式を変更してどちらのボタンで押したかの情報を入れる
	// もしくはoff→onへの切り替わりフレームかどうかをここで判定するようにして、その結果を入れる
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
			if (longEventY < longNoteY || longNoteY + longNote.length <= longEventY)
			{
				continue;
			}
			m_activeAudioEffectDictFX.emplace(audioEffectInvocation.audioEffectIdx, ksmaudio::AudioEffect::ActiveAudioEffectInvocation{
				.pOverrideParams = &audioEffectInvocation.overrideParams,
				.laneIdx = laneIdx,
			});
		}
	}

	AudioEffectMain::AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_longFXNoteInvocations((RegisterAudioEffects(bgm, chartData, timingCache), CreateLongFXNoteAudioEffectInvocations(bgm, chartData))) // 先に登録しておく必要があるので、分かりにくいがカンマ演算子を使用している(TODO: もうちょっとどうにかする)
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

		// TODO: laser
	}
}
