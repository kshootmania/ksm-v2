#include "ksmaudio/audio_effect/dsp/bitcrusher_dsp.hpp"
#include <cmath>

namespace ksmaudio::AudioEffect
{
	BitcrusherDSP::BitcrusherDSP(const DSPCommonInfo& info)
		: m_info(info)
	{
	}

	void BitcrusherDSP::process(float* pData, std::size_t dataSize, bool bypass, const BitcrusherDSPParams& params)
	{
		if (m_info.isUnsupported || bypass || params.mix == 0.0f)
		{
			return;
		}

		const float reduction = params.reduction * m_info.sampleRateScale;
		if (reduction == 0.0f)
		{
			return;
		}

		const std::size_t numFrames = dataSize / m_info.numChannels;
		float* pCursor = pData;
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			const bool updateHoldSample = (m_holdSampleCount > reduction);
			if (updateHoldSample)
			{
				m_holdSampleCount = std::fmod(m_holdSampleCount, reduction);
			}
			for (std::size_t channel = 0; channel < m_info.numChannels; ++channel)
			{
				if (updateHoldSample)
				{
					m_holdSample[channel] = *pCursor;
				}
				else
				{
					*pCursor = std::lerp(*pCursor, m_holdSample[channel], params.mix);
				}
				++pCursor;
			}
			m_holdSampleCount += 1.0f;
		}
	}
}
