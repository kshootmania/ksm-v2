#include "bgm.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr Duration kBlendTime = 5s;
		constexpr Duration kManualUpdateInterval = 0.005s;
	}

	void BGM::emplaceAudioEffectImpl(bool isFX, const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		if (m_stream.numChannels() == 0)
		{
			// ロード失敗時は音声エフェクトを追加しない
			return;
		}

		const auto pAudioEffectBus = isFX ? m_pAudioEffectBusFX : m_pAudioEffectBusLaser;
		switch (def.type)
		{
		case kson::AudioEffectType::Retrigger:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Retrigger>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Gate:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Gate>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Flanger:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Flanger>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::PitchShift:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::PitchShift>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Bitcrusher:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Bitcrusher>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Phaser:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Phaser>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Wobble:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Wobble>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Tapestop:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Tapestop>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Echo:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Echo>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::Sidechain:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::Sidechain>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::PeakingFilter:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::PeakingFilter>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::HighPassFilter:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::HighPassFilter>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		case kson::AudioEffectType::LowPassFilter:
			pAudioEffectBus->emplaceAudioEffect<ksmaudio::LowPassFilter>(name, def.v, paramChanges, updateTriggerTiming);
			break;

		default:
			assert(false && "Unknown audio effect type");
			break;
		}
	}

	BGM::BGM(FilePathView filePath, double volume, SecondsF offset)
		: m_stream(filePath.narrow(), volume, true, true)
		, m_duration(m_stream.duration())
		, m_offset(offset)
		, m_pAudioEffectBusFX(m_stream.emplaceAudioEffectBusFX())
		, m_pAudioEffectBusLaser(m_stream.emplaceAudioEffectBusLaser())
		, m_stopwatch(StartImmediately::No)
		, m_manualUpdateStopwatch(StartImmediately::Yes)
	{
	}

	void BGM::update()
	{
		if (m_isPaused)
		{
			return;
		}

		if (m_isStreamStarted)
		{
			if (m_manualUpdateStopwatch.elapsed() >= kManualUpdateInterval)
			{
				m_stream.updateManually();
				m_manualUpdateStopwatch.restart();
			}
			m_timeSec = m_stream.posSec() - m_offset;

			if (m_timeSec + m_offset < m_duration - kBlendTime)
			{
				// ストップウォッチの時間を同期
				m_stopwatch.set(SecondsF{ m_timeSec });
			}
		}
		else
		{
			m_timeSec = m_stopwatch.elapsed();

			if (m_timeSec + m_offset >= 0s)
			{
				m_stream.seekPosSec(m_timeSec + m_offset);
				m_stream.play();
				m_isStreamStarted = true;
			}
		}
	}

	void BGM::updateAudioEffectFX(bool bypass, const ksmaudio::AudioEffect::Status& status, const ksmaudio::AudioEffect::ActiveAudioEffectDict& activeAudioEffects)
	{
		m_pAudioEffectBusFX->setBypass(bypass);
		m_pAudioEffectBusFX->updateByFX(
			status,
			activeAudioEffects);
	}

	void BGM::updateAudioEffectLaser(bool bypass, const ksmaudio::AudioEffect::Status& status, const std::optional<std::size_t>& activeAudioEffectIdx)
	{
		m_pAudioEffectBusLaser->setBypass(bypass);
		m_pAudioEffectBusLaser->updateByLaser(
			status,
			activeAudioEffectIdx);
	}

	void BGM::play()
	{
		m_stopwatch.start();
		m_isStreamStarted = false;
		m_isPaused = false;
	}

	void BGM::pause()
	{
		if (m_isStreamStarted)
		{
			m_stream.pause();
		}
		m_stopwatch.pause();
		m_isPaused = true;
	}

	void BGM::seekPosSec(SecondsF posSec)
	{
		if (posSec < 0s)
		{
			m_stream.stop();
		}
		else
		{
			m_stream.seekPosSec(posSec);
		}
		m_timeSec = posSec;
		m_stopwatch.set(posSec);
	}

	SecondsF BGM::posSec() const
	{
		// 開始・終了地点でノーツの動きが一瞬止まるのを防ぐため、最初と最後は再生位置に対してストップウォッチの時間を混ぜる
		// TODO: うまく効いていないようなので見直す
		if (m_isStreamStarted)
		{
			const SecondsF timeSecWithOffset = m_timeSec + m_offset;
			if (0s <= timeSecWithOffset && timeSecWithOffset < kBlendTime)
			{
				const double lerpRate = timeSecWithOffset / kBlendTime;
				return Math::Lerp(m_stopwatch.elapsed(), m_timeSec, lerpRate);
			}
			else if (m_duration - kBlendTime <= timeSecWithOffset && timeSecWithOffset < m_duration)
			{
				const double lerpRate = (timeSecWithOffset - (m_duration - kBlendTime)) / kBlendTime;
				return Math::Lerp(m_timeSec, m_stopwatch.elapsed(), lerpRate);
			}
			else if (m_duration - m_offset <= m_stopwatch.elapsed())
			{
				return m_stopwatch.elapsed();
			}
		}

		return m_timeSec;
	}

	Duration BGM::duration() const
	{
		return m_duration;
	}

	Duration BGM::latency() const
	{
		return m_stream.latency();
	}

	void BGM::emplaceAudioEffectFX(const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		emplaceAudioEffectImpl(true, name, def, paramChanges, updateTriggerTiming);
	}

	void BGM::emplaceAudioEffectLaser(const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		emplaceAudioEffectImpl(false, name, def, paramChanges, updateTriggerTiming);
	}

	const ksmaudio::AudioEffect::AudioEffectBus& BGM::audioEffectBusFX() const
	{
		return *m_pAudioEffectBusFX;
	}

	const ksmaudio::AudioEffect::AudioEffectBus& BGM::audioEffectBusLaser() const
	{
		return *m_pAudioEffectBusLaser;
	}

	void BGM::setFadeOut(Duration duration)
	{
		m_stream.setFadeOut(duration);
	}
}
