#pragma once
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio
{
	class BGM
	{
	private:
		const ksmaudio::Stream/*s3d::Audio*/ m_audio;

		const double m_durationSec;

		double m_timeSec = 0.0;

		bool m_isStreamStarted = false;

		bool m_isPaused = true;

		Stopwatch m_stopwatch;

	public:
		explicit BGM(FilePathView filePath);

		void update();

		void play();

		void pause();

		void seekPosSec(double posSec);

		double posSec() const;
	};
}
