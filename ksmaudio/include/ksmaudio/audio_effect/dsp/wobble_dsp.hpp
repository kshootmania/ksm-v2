#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/wobble_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/detail/dsp_simple_trigger_handler.hpp"

namespace ksmaudio::AudioEffect
{
	class WobbleDSP
	{
	private:
		const DSPCommonInfo m_info;
		detail::DSPSimpleTriggerHandler m_triggerHandler;
		std::array<detail::BiquadFilter<float>, 2> m_lowPassFilters;

	public:
		explicit WobbleDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const WobbleDSPParams& params);

		void updateParams(const WobbleDSPParams& params);
	};
}
