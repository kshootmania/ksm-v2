#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/tapestop_params.hpp"
#include "ksmaudio/audio_effect/detail/time_modulator.hpp"

namespace ksmaudio::AudioEffect
{
	namespace detail
	{
        class TapeStopSpeedController
        {
        private:
			std::size_t m_framesSinceTrigger = 0U;
			float m_currentPlaySpeed = 1.0f;

        public:
            float nextSpeed(const TapestopDSPParams& params, float sampleRateScale);

			void reset();
        };
	}

	class TapestopDSP
	{
	private:
		const DSPCommonInfo m_info;
		detail::TapeStopSpeedController m_speedController;
		detail::TimeModulator m_timeModulator;

	public:
		explicit TapestopDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const TapestopDSPParams& params);
	};
}
