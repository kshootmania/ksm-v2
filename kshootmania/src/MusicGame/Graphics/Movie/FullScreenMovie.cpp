#include "FullScreenMovie.hpp"
#include "MovieUtils.hpp"

namespace MusicGame::Graphics
{
	FullScreenMovie::FullScreenMovie(const FilePath& moviePath, double movieOffsetSec, double playbackSpeed, bool loop, bool enabled)
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

		VideoReader videoReader;
		const auto loadedPath = MovieUtils::LoadWithFallback(moviePath, [&videoReader](const FilePath& path)
		{
			return videoReader.open(path) && videoReader.getFrameCount() > 0 && videoReader.getSize().x > 0 && videoReader.getSize().y > 0;
		});
		if (!loadedPath)
		{
			m_enabled = false;
			return;
		}

		m_size = videoReader.getSize();
		m_fps = 1.0 / videoReader.getFrameDeltaSec();
		m_frameCount = static_cast<int32>(videoReader.getFrameCount());
		m_fpsScale = MovieUtils::FpsScale(*loadedPath, videoReader.getFPS());
		m_frameReader = std::make_unique<MovieFrameReader>(videoReader, m_loop);
	}

	void FullScreenMovie::prepare(double globalOffsetSec)
	{
		m_startTimeSec = -m_movieOffsetSec - globalOffsetSec;
	}

	void FullScreenMovie::update(double currentTimeSec, bool isPaused)
	{
		if (!m_enabled || isPaused)
		{
			return;
		}

		const double moviePosSec = (currentTimeSec - m_startTimeSec) * m_playbackSpeed * m_fpsScale;
		double frameIndexDouble = Math::Floor(moviePosSec * m_fps);
		if (m_loop)
		{
			// 再生開始前も含めて繰り返す
			frameIndexDouble -= Math::Floor(frameIndexDouble / m_frameCount) * m_frameCount;
			m_isPlaying = true;
		}
		else
		{
			m_isPlaying = frameIndexDouble >= 0.0 && frameIndexDouble < m_frameCount;
		}

		// 再生開始前から先頭のフレームを読み込んでおく
		const int32 targetFrameIndex = static_cast<int32>(Clamp(frameIndexDouble, 0.0, static_cast<double>(m_frameCount - 1)));
		if (m_frameReader->fetch(targetFrameIndex, &m_frameImage))
		{
			m_hasUntransferredFrame = true;
		}

		if (m_hasUntransferredFrame)
		{
			auto& texture = m_frameTextures[1 - m_latestTextureIndex];
			bool transferred = false;
			if (texture)
			{
				transferred = texture.fillIfNotBusy(m_frameImage);
			}
			else
			{
				texture = DynamicTexture{ m_frameImage };
				transferred = static_cast<bool>(texture);
			}
			if (transferred)
			{
				m_latestTextureIndex = 1 - m_latestTextureIndex;
				m_hasUntransferredFrame = false;
				m_hasTexture = true;
			}
		}
	}

	bool FullScreenMovie::isEnabled() const
	{
		return m_enabled;
	}

	bool FullScreenMovie::hasFrame() const
	{
		return m_enabled && m_isPlaying && m_hasTexture;
	}

	const Texture& FullScreenMovie::texture() const
	{
		return m_frameTextures[m_latestTextureIndex];
	}

	Size FullScreenMovie::size() const
	{
		return m_size;
	}
}
