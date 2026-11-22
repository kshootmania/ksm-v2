#include "MoviePanel.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "MusicGame/Graphics/Movie/MovieUtils.hpp"

namespace MusicGame::Graphics
{
	MoviePanel::MoviePanel(const FilePath& moviePath, double movieOffsetSec, double playbackSpeed, bool loop, bool enabled)
		: m_movieOffsetSec(movieOffsetSec)
		, m_playbackSpeed(playbackSpeed)
		, m_loop(loop)
		, m_enabled(enabled)
	{
		if (!m_enabled)
		{
			return;
		}

		if (moviePath.isEmpty())
		{
			m_enabled = false;
			return;
		}

		const auto loadedPath = MovieUtils::LoadWithFallback(moviePath, [this](const FilePath& path)
		{
			m_movie = VideoTexture(path, Loop{ m_loop });
			return static_cast<bool>(m_movie);
		});
		if (!loadedPath)
		{
			m_enabled = false;
			return;
		}
		m_fpsScale = MovieUtils::FpsScale(*loadedPath, m_movie.getVideoReader().getFPS());
	}

	MoviePanel::~MoviePanel()
	{
		if (m_movie)
		{
			m_movie.release();
		}
	}

	void MoviePanel::prepare(double globalOffsetSec)
	{
		if (!m_enabled || !m_movie)
		{
			return;
		}

		m_startTimeSec = -m_movieOffsetSec - globalOffsetSec;
		m_started = false;
		m_prevLoopMoviePosSec = none;
	}

	void MoviePanel::update(double currentTimeSec, bool isPaused)
	{
		if (!m_enabled || !m_movie)
		{
			return;
		}

		if (m_loop)
		{
			if (isPaused)
			{
				return;
			}

			// 再生開始前も含めて繰り返す
			const double playbackSpeed = m_playbackSpeed != 0.0 ? m_playbackSpeed : 1.0;
			const double moviePosSec = (currentTimeSec - m_startTimeSec) * playbackSpeed * m_fpsScale;
			if (m_prevLoopMoviePosSec)
			{
				m_movie.advance(Max(moviePosSec - *m_prevLoopMoviePosSec, 0.0));
			}
			else if (const double lengthSec = m_movie.lengthSec(); lengthSec > 0.0)
			{
				m_movie.setPosSec(moviePosSec - Math::Floor(moviePosSec / lengthSec) * lengthSec);
				m_movie.advance(0.0);
			}
			m_prevLoopMoviePosSec = moviePosSec;
			m_started = true;
			return;
		}

		if (!m_started && currentTimeSec >= m_startTimeSec)
		{
			m_started = true;
		}

		if (m_started && !isPaused)
		{
			const double playbackSpeed = m_playbackSpeed != 0.0 ? m_playbackSpeed : 1.0;
			const double targetMoviePosSec = (currentTimeSec - m_startTimeSec) * playbackSpeed * m_fpsScale;
			const double currentMoviePosSec = m_movie.posSec();
			const double deltaSec = Max(targetMoviePosSec - currentMoviePosSec, 0.0);
			m_movie.advance(deltaSec);
		}
	}

	void MoviePanel::draw() const
	{
		if (!m_enabled || !m_movie)
		{
			return;
		}

		const double baseX = Scene::Width() / 2 + ScreenUtils::Scaled(kVideoOffsetFromCenterX);
		const double baseY = ScreenUtils::Scaled(kVideoBaseY);
		const Vec2 basePos{ baseX, baseY };
		const Vec2 targetSize = ScreenUtils::Scaled(Vec2{ kVideoWidth, kVideoHeight });

		// 外枠描画
		{
			const ScopedRenderStates2D blend(BlendState::Additive);
			const Vec2 frameSize = ScreenUtils::Scaled(Vec2{ kFrameWidth, kFrameHeight });
			const Vec2 frameCenter = basePos + Vec2{ targetSize.x / 2, targetSize.y / 2 };
			const Vec2 framePos = frameCenter - frameSize / 2;
			RectF{ framePos, frameSize }.draw(ColorF{ Palette::White, kFrameAlpha });
		}

		// 動画再生前は黒塗り、再生開始後は動画を描画
		if (!m_started)
		{
			// 黒塗り
			RectF{ basePos, targetSize }.draw(Palette::Black);
		}
		else
		{
			// 動画描画サイズ計算(アスペクト比を保持して短辺に合わせる)
			const Size videoSize = m_movie.size();
			const double videoAspect = static_cast<double>(videoSize.x) / videoSize.y;
			const double targetAspect = targetSize.x / targetSize.y;

			Vec2 drawSize;
			if (videoAspect > targetAspect)
			{
				// 動画が横長の場合は幅に合わせる
				drawSize = Vec2{ targetSize.x, targetSize.x / videoAspect };
			}
			else
			{
				// 動画が縦長の場合は高さに合わせる
				drawSize = Vec2{ targetSize.y * videoAspect, targetSize.y };
			}

			const Vec2 drawPos = basePos + (targetSize - drawSize) / 2;

			// 動画描画
			m_movie.resized(drawSize).draw(drawPos);
		}
	}

	bool MoviePanel::isEnabled() const
	{
		return m_enabled && m_movie;
	}

	void MoviePanel::seekPosSec(SecondsF posSec)
	{
		if (!m_enabled || !m_movie || !m_started)
		{
			return;
		}

		if (m_loop)
		{
			// 次の更新時に再生位置を設定し直す
			m_prevLoopMoviePosSec = none;
			return;
		}

		const double playbackSpeed = m_playbackSpeed != 0.0 ? m_playbackSpeed : 1.0;
		const double moviePosSec = (posSec.count() - m_startTimeSec) * playbackSpeed * m_fpsScale;
		if (moviePosSec >= 0.0)
		{
			m_movie.setPosSec(moviePosSec);
		}
	}
}
