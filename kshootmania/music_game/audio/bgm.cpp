#include "bgm.hpp"

MusicGame::Audio::BGM::BGM(FilePathView filePath)
	: m_audio(filePath)
	, m_stopwatch(StartImmediately::No)
{
}

void MusicGame::Audio::BGM::update()
{
	if (m_isPaused)
	{
		return;
	}

	if (m_audio.isPlaying())
	{
		const double rawTimeSec = m_audio.posSec();
		if (m_isPlayingPrev)
		{
			// Time filter (https://sol.gfxile.net/soloud/coremisc.html#soloud.getstreamtime) + delay removal
			m_timeSec = (m_timeSec * 9 + rawTimeSec) / 10 - m_delaySec;
			m_delaySec = (m_delaySec * 99 + (m_timeSec - rawTimeSec)) / 100;
		}
		else
		{
			m_timeSec = rawTimeSec;
			m_delaySec = 0.0;
		}

		m_isPlayingPrev = true;
	}
	else
	{
		if (m_isPlayingPrev || !m_stopwatch.isStarted())
		{
			m_stopwatch.set(SecondsF(m_timeSec));
			m_stopwatch.resume();
		}
		else
		{
			m_timeSec = m_stopwatch.sF();
		}
		m_delaySec = 0.0;

		if (m_timeSec >= 0.0)
		{
			m_audio.seekTime(m_timeSec);
			m_audio.play();
		}

		m_isPlayingPrev = false;
	}
}

void MusicGame::Audio::BGM::play()
{
	m_isPaused = false;
}

void MusicGame::Audio::BGM::pause()
{
	m_isPaused = true;
}

void MusicGame::Audio::BGM::seekTime(double posSec)
{
	if (posSec < 0.0)
	{
		m_audio.stop();
	}
	else
	{
		m_audio.seekTime(posSec);
	}
	m_timeSec = posSec;
	m_delaySec = 0.0;
}

double MusicGame::Audio::BGM::posSec() const
{
	// Blend stopwatch time to remove brake at start
	constexpr double kBlendMaxTimeSec = 5.0;
	if (m_audio.isPlaying() && 0.0 <= m_timeSec && m_timeSec < kBlendMaxTimeSec)
	{
		const double lerpRate = m_timeSec / kBlendMaxTimeSec;
		return Math::Lerp(m_stopwatch.sF(), m_timeSec, lerpRate);
	}

	return m_timeSec;
}
