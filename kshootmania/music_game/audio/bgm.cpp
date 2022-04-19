#include "bgm.hpp"

namespace
{
	constexpr double kRelaxationTime = 10.0 / 60;
	constexpr double kDelayRelaxationTime = 100.0 / 60;
}

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
		const double dt = Scene::DeltaTime();
		if (m_isPlayingPrev && dt < kRelaxationTime)
		{
			// Time filter (https://sol.gfxile.net/soloud/coremisc.html#soloud.getstreamtime) + delay removal
			m_timeSec = (m_timeSec * (kRelaxationTime - dt) + rawTimeSec * dt) / kRelaxationTime;
			m_delaySec = (m_delaySec * (kDelayRelaxationTime - dt) + (m_timeSec - rawTimeSec) * dt) / kDelayRelaxationTime;
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
			m_stopwatch.set(SecondsF(m_timeSec - m_delaySec));
			m_stopwatch.resume();
		}
		else
		{
			m_timeSec = m_stopwatch.sF();
		}
		m_delaySec = 0.0;

		if (m_timeSec >= 0.0 && m_timeSec < m_audio.lengthSec())
		{
			m_audio.seekTime(m_timeSec);
			m_audio.play();
		}

		m_isPlayingPrev = false;
	}

	Print << m_timeSec;
}

void MusicGame::Audio::BGM::play()
{
	m_isPaused = false;
	m_isPlayingPrev = false;

	if (m_audio.isPaused())
	{
		m_audio.play();
	}
}

void MusicGame::Audio::BGM::pause()
{
	m_isPaused = true;

	if (m_audio.isPlaying())
	{
		m_audio.pause();
	}
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
	const double timeSec = m_timeSec - m_delaySec;
	constexpr double kBlendMaxTimeSec = 5.0;
	if (m_audio.isPlaying() && 0.0 <= timeSec && timeSec < kBlendMaxTimeSec)
	{
		const double lerpRate = timeSec / kBlendMaxTimeSec;
		return Math::Lerp(m_stopwatch.sF(), timeSec, lerpRate);
	}

	return timeSec;
}
