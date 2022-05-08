#include "bgm.hpp"

namespace
{
	constexpr double kRelaxationTime = 10.0 / 60;
	constexpr double kDelayRelaxationTime = 100.0 / 60;
	constexpr double kBlendTimeSec = 5.0;
}

MusicGame::Audio::BGM::BGM(FilePathView filePath)
	: m_audio(filePath.toUTF8())
	, m_durationSec(m_audio.durationSec())
	, m_stopwatch(StartImmediately::No)
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
		m_timeSec = m_audio.posSec();

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
			m_audio.seekPosSec(m_timeSec);
			m_audio.play();
			m_isStreamStarted = true;
		}
	}
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
		m_audio.pause();
	}
	m_stopwatch.pause();
	m_isPaused = true;
}

void MusicGame::Audio::BGM::seekPosSec(double posSec)
{
	if (posSec < 0.0)
	{
		m_audio.stop();
	}
	else
	{
		m_audio.seekPosSec(posSec);
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
