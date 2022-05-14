#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/flanger_params.hpp"
#include "ksmaudio/audio_effect/detail/ring_buffer.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"

namespace ksmaudio::AudioEffect
{
	class FlangerDSP
	{
	private:
		const DSPCommonInfo m_info;
		detail::RingBuffer<float> m_ringBuffer;
		float m_lfoTimeRate = 0.0f;
		std::array<detail::BiquadFilter<float>, 2> m_lowShelfFilters;

	public:
		explicit FlangerDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const FlangerDSPParams& params);
	};
}
