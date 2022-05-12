#pragma once
#include <memory>
#include "bass.h"

namespace ksmaudio
{

	class IAudioEffect
	{
	public:
		virtual ~IAudioEffect() = default;

		virtual void process(float* pData, std::size_t dataSize) = 0;
	};

}
