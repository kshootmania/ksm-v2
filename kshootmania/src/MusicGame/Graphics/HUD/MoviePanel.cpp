#include "MoviePanel.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"

namespace MusicGame::Graphics
{
	MoviePanel::MoviePanel(const FilePath& moviePath, double movieOffsetSec, double playbackSpeed, bool enabled)
		: m_movieOffsetSec(movieOffsetSec)
		, m_playbackSpeed(playbackSpeed)
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

		if (FileSystem::Exists(moviePath))
		{
			m_movie = VideoTexture(moviePath, Loop::No);
			if (m_movie)
			{
				return;
			}
		}

		// 読み込み失敗または存在しない場合、拡張子が.mp4でなければ.mp4に変えて再試行
		if (FileSystem::Extension(moviePath) != U"mp4")
		{
			const FilePath mp4Path = FileSystem::PathAppend(FileSystem::ParentPath(moviePath), FileSystem::BaseName(moviePath) + U".mp4");
			if (FileSystem::Exists(mp4Path))
			{
				Logger << U"[ksm info] Trying alternative movie file: {}"_fmt(mp4Path);
				m_movie = VideoTexture(mp4Path, Loop::No);
				if (m_movie)
				{
					return;
				}
			}
		}

		Logger << U"[ksm warning] Failed to load movie file: {}"_fmt(moviePath);
		m_enabled = false;
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
	}

	void MoviePanel::update(double currentTimeSec, bool isPaused)
	{
		if (!m_enabled || !m_movie)
		{
			return;
		}

		if (!m_started && currentTimeSec >= m_startTimeSec)
		{
			m_started = true;
		}

		if (m_started && !isPaused)
		{
			const double targetMoviePosSec = currentTimeSec - m_startTimeSec;
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

		const double moviePosSec = posSec.count() - m_startTimeSec;
		if (moviePosSec >= 0.0)
		{
			m_movie.setPosSec(moviePosSec);
		}
	}
}
