#pragma once
#include <array>
#include "audio_effect_dsp.hpp"
#include "ring_buffer.hpp"
#include "biquad_filter.hpp"

namespace ksmaudio
{
	struct FlangerParams
	{
		float periodSec = 4.0f;
		float delay = 30.0f;
		float depth = 45.0f;
		float feedback = 0.6f;
		float stereoWidth = 0.0f;
		float vol = 0.75f;
		float mix = 0.8f;
	};

	class FlangerDSP : public IAudioEffectDSP
	{
	private:
		const std::size_t m_sampleRate;

		const float m_sampleRateScale;

		const std::size_t m_numChannels;

		const bool m_isUnsupported;

		RingBuffer<float> m_ringBuffer;

		float m_lfoTimeRate = 0.0f;

		std::array<BiquadFilter<float>, 2> m_lowShelfFilters;

		FlangerParams m_params;

		bool m_bypass = false;

	public:
		FlangerDSP(std::size_t sampleRate, std::size_t numChannels, const FlangerParams& params);

		virtual void process(float* pData, std::size_t dataSize) override;

		virtual bool getBypass() const override;

		virtual void setBypass(bool b) override;

		void setParams(const FlangerParams& params);
	};

}
