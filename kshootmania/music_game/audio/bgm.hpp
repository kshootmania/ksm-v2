#pragma once

namespace MusicGame::Audio
{
	class BGM
	{
	private:
		s3d::Audio m_audio;

		double m_delaySec = 0.0;

		double m_timeSec = 0.0;

		bool m_isPaused = true;

		bool m_isPlayingPrev = false;

		Stopwatch m_stopwatch;

	public:
		explicit BGM(FilePathView filePath);

		void update();

		void play();

		void pause();

		void seekTime(double posSec);

		double posSec() const;
	};
}
