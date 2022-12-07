#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/low_pass_filter_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/detail/linear_easing.hpp"

namespace ksmaudio::AudioEffect
{
	class LowPassFilterDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<detail::BiquadFilter<float>, 2> m_lowPassFilters;
		float m_prevV = 0.0f;
		detail::LinearEasing<float> m_freqEasing; // TODO: freqではなくvに適用

	public:
		explicit LowPassFilterDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const LowPassFilterDSPParams& params);
	};
}
