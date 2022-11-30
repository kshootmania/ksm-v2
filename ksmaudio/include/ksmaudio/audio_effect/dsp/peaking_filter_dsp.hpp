#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/peaking_filter_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"

namespace ksmaudio::AudioEffect
{
	class PeakingFilterDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<detail::BiquadFilter<float>, 2> m_peakingFilters;
		float m_prevV = 0.0f;

	public:
		explicit PeakingFilterDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params);
	};
}
