#include "ksmaudio/audio_effect/dsp/peaking_filter_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	PeakingFilterDSP::PeakingFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
	{
	}

	void PeakingFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		const float fSampleRate = static_cast<float>(m_info.sampleRate);
		for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
		{
			m_peakingFilters[ch].setPeakingFilter(params.freq, params.bandwidth, params.gain, fSampleRate);
		}

		const bool isBypassed = bypass || params.mix == 0.0f; // Ø‚è‘Ö‚¦Žž‚ÌƒmƒCƒY‰ñ”ð‚Ì‚½‚ß‚Ébypassó‘Ô‚Å‚àˆ—Ž©‘Ì‚Í‚·‚é

		assert(dataSize % m_info.numChannels == 0);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				const float wet = m_peakingFilters[ch].process(*pData);
				if (!isBypassed)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}
		}
	}
}
