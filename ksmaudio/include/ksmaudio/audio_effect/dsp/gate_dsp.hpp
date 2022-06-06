#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/gate_params.hpp"

namespace ksmaudio::AudioEffect
{
	class GateDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::ptrdiff_t m_framesUntilTrigger = -1;
		std::size_t m_framesSincePrevTrigger = 0U;

	public:
		explicit GateDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const GateDSPParams& params);
	};
}
