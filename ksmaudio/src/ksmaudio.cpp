#include "ksmaudio/ksmaudio.hpp"
#include "bass.h"

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
