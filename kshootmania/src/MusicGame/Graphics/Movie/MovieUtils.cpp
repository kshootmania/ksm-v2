#include "MovieUtils.hpp"

#ifdef _WIN32
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#endif

namespace MusicGame::Graphics::MovieUtils
{
	namespace
	{
#ifdef _WIN32
		// Media Foundationを直接叩いてフレームのプレゼンテーション時刻から平均フレームレートを取得
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

	Optional<FilePath> LoadWithFallback(const FilePath& moviePath, const std::function<bool(const FilePath&)>& fnLoad)
	{
		if (FileSystem::Exists(moviePath) && fnLoad(moviePath))
		{
			return moviePath;
		}

		if (FileSystem::Extension(moviePath) != U"mp4")
		{
			const FilePath mp4Path = FileSystem::PathAppend(FileSystem::ParentPath(moviePath), FileSystem::BaseName(moviePath) + U".mp4");
			if (FileSystem::Exists(mp4Path))
			{
				Logger << U"[ksm info] Trying alternative movie file: {}"_fmt(mp4Path);
				if (fnLoad(mp4Path))
				{
					return mp4Path;
				}
			}
		}

		Logger << U"[ksm warning] Failed to load movie file: {}"_fmt(moviePath);
		return none;
	}

	double FpsScale([[maybe_unused]] const FilePath& loadedPath, [[maybe_unused]] double reportedFps)
	{
#ifdef _WIN32
		if (FileSystem::Extension(loadedPath) == U"wmv")
		{
			if (const auto actualFps = MeasureFrameRateFromPTS(loadedPath); actualFps && *actualFps > 0.0 && reportedFps > 0.0)
			{
				if (Abs(*actualFps - reportedFps) > 0.001)
				{
					const double scale = *actualFps / reportedFps;
					if (scale > 0.001 && scale < 1000.0)
					{
						return scale;
					}
				}
			}
		}
#endif
		return 1.0;
	}
}
