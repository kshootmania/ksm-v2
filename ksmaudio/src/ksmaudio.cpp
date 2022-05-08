#include "ksmaudio/ksmaudio.hpp"
#include "bass.h"

namespace
{
	constexpr DWORD kSampleRate = 44100;
	constexpr DWORD kBufferSizeMs = 200;
	constexpr DWORD kUpdatePeriodMs = 100;
	constexpr DWORD kUpdateThreads = 2;
}

namespace ksmaudio
{
	void Init(void* hWnd)
	{
		BASS_Init(-1/* default device */, kSampleRate, 0, static_cast<HWND>(hWnd), nullptr);
		BASS_SetConfig(BASS_CONFIG_BUFFER, kBufferSizeMs);
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, kUpdatePeriodMs);
		BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
		BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, kUpdateThreads);
	}

	void Terminate()
	{
		BASS_Free();
	}
}
