#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/high_pass_filter_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/detail/linear_easing.hpp"

namespace ksmaudio::AudioEffect
{
	class HighPassFilterDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<detail::BiquadFilter<float>, 2> m_highPassFilters;
		detail::LinearEasing<float> m_vEasing;

	public:
		explicit HighPassFilterDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const HighPassFilterDSPParams& params, bool isParamUpdated);
	};
}
