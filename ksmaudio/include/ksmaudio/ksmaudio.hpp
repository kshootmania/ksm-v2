#pragma once
#include "stream.hpp"
#include "stream_with_effects.hpp"
#include "sample.hpp"
#include "audio_effect/all.hpp"

namespace ksmaudio
{
	constexpr DWORD kSampleRate = 44100;
	constexpr DWORD kBufferSizeMs = 200;
	constexpr DWORD kUpdatePeriodMs = 100;
	constexpr DWORD kUpdateThreads = 2;

	void Init(void* hWnd);

	void Terminate();
}
