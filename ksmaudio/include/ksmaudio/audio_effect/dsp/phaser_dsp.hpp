#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/phaser_params.hpp"
#include "ksmaudio/audio_effect/detail/ring_buffer.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"

namespace ksmaudio::AudioEffect
{
	class PhaserDSP
	{
	public:
		static constexpr std::size_t kMaxNumAllPassFilters = 12U;

	private:
		const DSPCommonInfo m_info;
		float m_lfoTimeRate = 0.0f;
		std::array<std::array<detail::BiquadFilter<float>, 2>, kMaxNumAllPassFilters> m_allPassFilters;
		std::array<detail::BiquadFilter<float>, 2> m_hiCutFilters;
		std::array<std::array<float, 2>, kMaxNumAllPassFilters> m_prevWetArrayForFeedback;

	public:
		explicit PhaserDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const PhaserDSPParams& params);

		void updateParams(const PhaserDSPParams& params);
	};
}
