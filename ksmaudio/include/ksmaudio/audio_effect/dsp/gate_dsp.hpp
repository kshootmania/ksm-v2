#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/gate_params.hpp"
#include "ksmaudio/audio_effect/detail/dsp_simple_trigger_handler.hpp"

namespace ksmaudio::AudioEffect
{
	class GateDSP
	{
	private:
		const DSPCommonInfo m_info;
		detail::DSPSimpleTriggerHandler m_triggerHandler;

	public:
		explicit GateDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const GateDSPParams& params);

		void updateParams(const GateDSPParams& params);
	};
}
