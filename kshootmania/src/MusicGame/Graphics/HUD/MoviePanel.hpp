#pragma once
#include <Siv3D.hpp>

namespace MusicGame::Graphics
{
	class MoviePanel
	{
	private:
		VideoTexture m_movie;
		bool m_started = false;
		double m_movieOffsetSec = 0.0;
		double m_startTimeSec = 0.0;
		double m_playbackSpeed = 1.0;
		double m_fpsScale = 1.0;
		bool m_loop = false;
		Optional<double> m_prevLoopMoviePosSec; // ループ再生で前回の更新時に求めた再生位置(折り返す前の値)
		bool m_enabled = false;

		static constexpr int32 kVideoOffsetFromCenterX = -302;
		static constexpr int32 kVideoBaseY = 140;
		static constexpr int32 kVideoWidth = 160;
		static constexpr int32 kVideoHeight = 120;

		static constexpr int32 kFrameWidth = 170;
		static constexpr int32 kFrameHeight = 130;
		static constexpr double kFrameAlpha = 96.0 / 255.0;

	public:
		MoviePanel(const FilePath& moviePath, double movieOffsetSec, double playbackSpeed, bool loop, bool enabled);

		~MoviePanel();

		void prepare(double globalOffsetSec);

		void update(double currentTimeSec, bool isPaused);

		void draw() const;

		[[nodiscard]]
		bool isEnabled() const;

		void seekPosSec(SecondsF posSec);
	};
}
