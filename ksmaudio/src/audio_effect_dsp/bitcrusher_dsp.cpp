#include "ksmaudio/audio_effect_dsp/bitcrusher_dsp.hpp"
#include <cmath>

namespace ksmaudio
{

	BitcrusherDSP::BitcrusherDSP(std::size_t sampleRate, std::size_t numChannels, const BitcrusherParams& params)
		: m_sampleRateScale(static_cast<float>(sampleRate) / 44100.0f)
		, m_numChannels(numChannels)
		, m_isUnsupported(m_numChannels == 0U || m_numChannels >= 3U) // Supports stereo and mono only
	{
	}

	void BitcrusherDSP::process(float* pData, std::size_t dataSize)
	{
		if (m_bypass || m_isUnsupported)
		{
			return;
		}

		const std::size_t numFrames = dataSize / m_numChannels;
		const float reduction = m_params.reduction * m_sampleRateScale;
		float* pCursor = pData;
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			const bool updateHoldSample = (m_holdSampleCount > reduction);
			if (updateHoldSample)
			{
				m_holdSampleCount = std::fmod(m_holdSampleCount, reduction);
			}
			for (std::size_t channel = 0; channel < m_numChannels; ++channel)
			{
				if (updateHoldSample)
				{
					m_holdSample[channel] = *pCursor;
				}
				else
				{
					*pCursor = m_holdSample[channel];
				}
				++pCursor;
			}
			m_holdSampleCount += 1.0f;
		}
	}

	bool BitcrusherDSP::getBypass() const
	{
		return m_bypass;
	}

	void BitcrusherDSP::setBypass(bool b)
	{
		m_bypass = b;
	}

	void BitcrusherDSP::setParams(const BitcrusherParams& params)
	{
		m_params = params;
	}

}
