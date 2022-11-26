#include "bgm.hpp"

namespace
{
	constexpr double kBlendTimeSec = 5.0;
	constexpr double kManualUpdateIntervalSec = 0.005;
}

void MusicGame::Audio::BGM::emplaceAudioEffectImpl(bool isFX, const std::string& name, const kson::AudioEffectDef& def, const std::set<float>& updateTriggerTiming)
{
	const auto pAudioEffectBus = isFX ? m_pAudioEffectBusFX : m_pAudioEffectBusLaser;
	switch (def.type)
	{
	case kson::AudioEffectType::Retrigger:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Retrigger>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Gate:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Gate>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Flanger:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Flanger>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Bitcrusher:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Bitcrusher>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Wobble:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Wobble>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Tapestop:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Tapestop>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::Echo:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::Echo>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;

	case kson::AudioEffectType::PeakingFilter:
		pAudioEffectBus->emplaceAudioEffect<ksmaudio::PeakingFilter>(name, def.v, { /*TODO*/ }, updateTriggerTiming);
		break;
	}
}

MusicGame::Audio::BGM::BGM(FilePathView filePath)
	: m_stream(filePath.toUTF8())
	, m_durationSec(m_stream.durationSec())
	, m_pAudioEffectBusFX(m_stream.emplaceAudioEffectBus())
	, m_pAudioEffectBusLaser(m_stream.emplaceAudioEffectBus())
	, m_stopwatch(StartImmediately::No)
	, m_manualUpdateStopwatch(StartImmediately::Yes)
{
}

void MusicGame::Audio::BGM::update()
{
	if (m_isPaused)
	{
		return;
	}

	if (m_isStreamStarted)
	{
		if (m_manualUpdateStopwatch.sF() >= kManualUpdateIntervalSec)
		{
			m_stream.updateManually();
			m_manualUpdateStopwatch.restart();
		}
		m_timeSec = m_stream.posSec();

		if (m_timeSec < m_durationSec - kBlendTimeSec)
		{
			// ストップウォッチの時間を同期
			m_stopwatch.set(SecondsF{ m_timeSec });
		}
	}
	else
	{
		m_timeSec = m_stopwatch.sF();

		if (m_timeSec >= 0.0)
		{
			m_stream.seekPosSec(m_timeSec);
			m_stream.play();
			m_isStreamStarted = true;
		}
	}
}

void MusicGame::Audio::BGM::updateAudioEffectFX(bool bypass, const ksmaudio::AudioEffect::Status& status, const ksmaudio::AudioEffect::ActiveAudioEffectDict& activeAudioEffects)
{
	m_pAudioEffectBusFX->setBypass(bypass);
	m_pAudioEffectBusFX->update(
		status,
		activeAudioEffects);
}

void MusicGame::Audio::BGM::updateAudioEffectLaser(bool bypass, const ksmaudio::AudioEffect::Status& status, const std::optional<std::size_t>& activeAudioEffectIdx)
{
	m_pAudioEffectBusLaser->setBypass(bypass);
	m_pAudioEffectBusLaser->update(
		status,
		activeAudioEffectIdx);
}

void MusicGame::Audio::BGM::play()
{
	m_stopwatch.start();
	m_isStreamStarted = false;
	m_isPaused = false;
}

void MusicGame::Audio::BGM::pause()
{
	if (m_isStreamStarted)
	{
		m_stream.pause();
	}
	m_stopwatch.pause();
	m_isPaused = true;
}

void MusicGame::Audio::BGM::seekPosSec(double posSec)
{
	if (posSec < 0.0)
	{
		m_stream.stop();
	}
	else
	{
		m_stream.seekPosSec(posSec);
	}
	m_timeSec = posSec;
	m_stopwatch.set(SecondsF{ posSec });
}

double MusicGame::Audio::BGM::posSec() const
{
	// 開始・終了地点でノーツの動きが一瞬止まるのを防ぐため、最初と最後は再生位置に対してストップウォッチの時間を混ぜる
	// TODO: うまく効いていないようなので見直す
	if (m_isStreamStarted)
	{
		if (0.0 <= m_timeSec && m_timeSec < kBlendTimeSec)
		{
			const double lerpRate = m_timeSec / kBlendTimeSec;
			return Math::Lerp(m_stopwatch.sF(), m_timeSec, lerpRate);
		}
		else if (m_durationSec - kBlendTimeSec <= m_timeSec && m_timeSec < m_durationSec)
		{
			const double lerpRate = (m_timeSec - (m_durationSec - kBlendTimeSec)) / kBlendTimeSec;
			return Math::Lerp(m_timeSec, m_stopwatch.sF(), lerpRate);
		}
		else if (m_durationSec <= m_stopwatch.sF())
		{
			return m_stopwatch.sF();
		}
	}

	return m_timeSec;
}

double MusicGame::Audio::BGM::durationSec() const
{
	return m_durationSec;
}

double MusicGame::Audio::BGM::latencySec() const
{
	return m_stream.latencySec();
}

void MusicGame::Audio::BGM::emplaceAudioEffectFX(const std::string& name, const kson::AudioEffectDef& def, const std::set<float>& updateTriggerTiming)
{
	emplaceAudioEffectImpl(true, name, def, updateTriggerTiming);
}

void MusicGame::Audio::BGM::emplaceAudioEffectLaser(const std::string& name, const kson::AudioEffectDef& def, const std::set<float>& updateTriggerTiming)
{
	emplaceAudioEffectImpl(false, name, def, updateTriggerTiming);
}

const ksmaudio::AudioEffect::AudioEffectBus& MusicGame::Audio::BGM::audioEffectBusFX() const
{
	return *m_pAudioEffectBusFX;
}

const ksmaudio::AudioEffect::AudioEffectBus& MusicGame::Audio::BGM::audioEffectBusLaser() const
{
	return *m_pAudioEffectBusLaser;
}
