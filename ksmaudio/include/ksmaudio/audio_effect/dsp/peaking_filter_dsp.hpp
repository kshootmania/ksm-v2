#pragma once
#include <optional>
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/peaking_filter_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/detail/linear_easing.hpp"

namespace ksmaudio::AudioEffect
{
	namespace detail
	{
		class PeakingFilterValueController
		{
		private:
			detail::LinearEasing<float> m_vEasing;
			std::optional<float> m_prevVInGainUpdate = std::nullopt;
			float m_baseGainDb = 0.0f;
			float m_freq = 1000.0f;
			bool m_updated = true;

		public:
			PeakingFilterValueController();

			void updateGain(float v);

			void updateFreq(float v);

			bool popUpdated();

			float baseGainDb() const;

			float freq() const;

			bool mixSkipped() const;
		};
	}

	class PeakingFilterDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<detail::BiquadFilter<float>, 2> m_peakingFilters;
		detail::PeakingFilterValueController m_valueController;

	public:
		explicit PeakingFilterDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params);

		void updateParams(const PeakingFilterDSPParams& params);
	};
}
