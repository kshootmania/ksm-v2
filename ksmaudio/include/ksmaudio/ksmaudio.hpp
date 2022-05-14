#pragma once

namespace ksmaudio
{
	void Init(void* hWnd);

	void Terminate();
}

#include "stream.hpp"
#include "stream_with_effects.hpp"
#include "sample.hpp"
#include "audio_effect/all.hpp"
