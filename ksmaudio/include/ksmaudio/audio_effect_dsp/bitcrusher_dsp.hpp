#pragma once
#include <array>
#include "audio_effect_dsp.hpp"

namespace ksmaudio
{
	struct BitcrusherParams
	{
		float reduction = 10.0f;
		float mix = 1.0f;
	};

	class BitcrusherDSP : public IAudioEffectDSP
	{
	private:
		const float m_sampleRateScale;

		const std::size_t m_numChannels;

		const bool m_isUnsupported;

		std::array<float, 2> m_holdSample;

		float m_holdSampleCount = 0.0f;

		BitcrusherParams m_params;

		bool m_bypass = false;

	public:
		BitcrusherDSP(std::size_t sampleRate, std::size_t numChannels, const BitcrusherParams& params);

		virtual void process(float* pData, std::size_t dataSize) override;

		virtual bool getBypass() const override;

		virtual void setBypass(bool b) override;

		void setParams(const BitcrusherParams& params);
	};

}
