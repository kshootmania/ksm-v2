#include "MoviePanel.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"

#ifdef _WIN32
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#endif

namespace MusicGame::Graphics
{
	namespace
	{
#ifdef _WIN32
		// Media Foundationを直接叩いてフレームのプレゼンテーション時刻から平均フレームレートを取得
		// (OpenSiv3DのVideoReaderがwmvを本来と異なる30fpsとして報告する場合があるため、その場合の補正用)
		Optional<double> MeasureFrameRateFromPTS(const FilePath& path)
		{
			using Microsoft::WRL::ComPtr;

			if (FAILED(MFStartup(MF_VERSION)))
			{
				return none;
			}

			ComPtr<IMFSourceReader> pReader;
			const std::wstring wpath = path.toWstr();
			if (FAILED(MFCreateSourceReaderFromURL(wpath.c_str(), nullptr, &pReader)) || !pReader)
			{
				return none;
			}

			Array<int64> framePts;
			constexpr int32 kSampleFrameCount = 300;
			for (int32 i = 0; i < kSampleFrameCount; ++i)
			{
				DWORD streamIndex = 0;
				DWORD flags = 0;
				LONGLONG pts = 0;
				ComPtr<IMFSample> pSample;

				const HRESULT hr = pReader->ReadSample(
					static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
					0,
					&streamIndex,
					&flags,
					&pts,
					&pSample);
				if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM))
				{
					break;
				}
				framePts.push_back(static_cast<int64>(pts));
			}

			if (framePts.size() < 2)
			{
				return none;
			}
			framePts.sort();

			const int64 totalDelta = framePts.back() - framePts.front();
			const int64 intervalCount = static_cast<int64>(framePts.size()) - 1;
			if (totalDelta <= 0 || intervalCount <= 0)
			{
				return none;
			}
			const double avgDeltaSec = static_cast<double>(totalDelta) / 10000000.0 / static_cast<double>(intervalCount);
			if (avgDeltaSec <= 0.0)
			{
				return none;
			}
			return 1.0 / avgDeltaSec;
		}
#endif
	}

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

		const auto applyFpsScale = [this](const FilePath& loadedPath)
		{
#ifdef _WIN32
			const String ext = FileSystem::Extension(loadedPath);
			if (ext == U"wmv")
			{
				const double reportedFps = m_movie.getVideoReader().getFPS();
				if (const auto actualFps = MeasureFrameRateFromPTS(loadedPath); actualFps && *actualFps > 0.0 && reportedFps > 0.0)
				{
					if (Abs(*actualFps - reportedFps) > 0.001)
					{
						const double scale = *actualFps / reportedFps;
						if (scale > 0.001 && scale < 1000.0)
						{
							m_fpsScale = scale;
						}
					}
				}
			}
#else
			(void)loadedPath;
#endif
		};

		if (FileSystem::Exists(moviePath))
		{
			m_movie = VideoTexture(moviePath, Loop::No);
			if (m_movie)
			{
				applyFpsScale(moviePath);
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
					applyFpsScale(mp4Path);
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
			const double targetMoviePosSec = (currentTimeSec - m_startTimeSec) * m_fpsScale;
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

		const double moviePosSec = (posSec.count() - m_startTimeSec) * m_fpsScale;
		if (moviePosSec >= 0.0)
		{
			m_movie.setPosSec(moviePosSec);
		}
	}
}
