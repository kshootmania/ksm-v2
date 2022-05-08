#pragma once
#include "bass.h"

namespace ksmaudio
{

	class IAudioEffectDSP
	{
	public:
		virtual ~IAudioEffectDSP() = default;

		virtual void process(float* pData, std::size_t dataSize) = 0;

		virtual bool getBypass() const = 0;

		virtual void setBypass(bool b) = 0;
	};

}
