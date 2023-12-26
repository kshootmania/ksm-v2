#include "ksmaudio/ksmaudio.hpp"
#include "bass.h"

namespace ksmaudio
{
	void Init(void* hWnd)
	{
#ifdef _WIN32
		BASS_Init(-1/* default device */, kSampleRate, 0, static_cast<HWND>(hWnd), nullptr);
#else
		(void)hWnd;
		BASS_Init(-1/* default device */, kSampleRate, 0, 0, nullptr);
#endif
		BASS_SetConfig(BASS_CONFIG_BUFFER, kBufferSizeMs);
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, kUpdatePeriodMs);
		BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
		BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, kUpdateThreads);

		BASS_FX_GetVersion(); // bass_fx.dllをロードするために呼ぶ必要あり
	}

	void Terminate()
	{
		BASS_Free();
	}
}
