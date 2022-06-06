#include "bgm.hpp"

namespace
{
	constexpr double kBlendTimeSec = 5.0;
	constexpr double kManualUpdateIntervalSec = 0.005;
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
			// Synchronize stopwatch value
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

void MusicGame::Audio::BGM::updateAudioEffectFX(const ksmaudio::AudioEffect::Status& status, const kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict>& activeAudioEffects)
{
	m_pAudioEffectBusFX->update(
		status,
		activeAudioEffects);
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
	// Blend stopwatch time to remove brake at start/end
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

void MusicGame::Audio::BGM::emplaceAudioEffect(bool isFX, const std::string& name, const kson::AudioEffectDef& def, const std::set<float>& updateTriggerTiming)
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
	}
}
