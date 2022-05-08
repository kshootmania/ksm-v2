#include "ksmaudio/audio_effect_dsp/flanger_dsp.hpp"

namespace ksmaudio
{

	FlangerDSP::FlangerDSP(std::size_t sampleRate, std::size_t numChannels, const FlangerParams& params)
		: m_sampleRate(sampleRate)
		, m_sampleRateScale(static_cast<float>(sampleRate) / 44100.0f)
		, m_numChannels(numChannels)
		, m_isUnsupported(m_numChannels == 0U || m_numChannels >= 3U) // Supports stereo and mono only
		, m_ringBuffer(
			static_cast<std::size_t>(sampleRate) * 3 * numChannels, // 3 seconds
			numChannels)
		, m_params(params)
	{
		for (int i = 0; i < 2; ++i)
		{
			m_lowShelfFilters[i].setLowShelfFilter(250.0f, 0.5f, -20.0f, static_cast<float>(m_sampleRate));
		}
	}

	void FlangerDSP::process(float* pData, std::size_t dataSize)
	{
		if (m_isUnsupported || dataSize > m_ringBuffer.size())
		{
			return;
		}

		assert(dataSize % m_numChannels == 0);
		const std::size_t numFrames = dataSize / m_numChannels;
		if (m_bypass)
		{
			m_ringBuffer.write(pData, dataSize);
			m_ringBuffer.advanceCursor(numFrames);
			return;
		}

		const float lfoSpeed = 1.0f / m_params.periodSec / m_sampleRate;
		float* pCursor = pData;
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			for (std::size_t channel = 0; channel < m_numChannels; ++channel)
			{
				const float lfoValue = (channel == 0U) ? Triangle(m_lfoTimeRate) : Triangle(DecimalPart(m_lfoTimeRate + m_params.stereoWidth / 2));
				const float delayFrames = (m_params.delay + lfoValue * m_params.depth) * m_sampleRateScale;
				const float wet = (*pCursor + m_lowShelfFilters[channel].process(m_ringBuffer.lerpedDelay(delayFrames, channel))) * m_params.vol;
				m_ringBuffer.write(Lerp(*pCursor, wet, m_params.feedback), channel);
				*pCursor = Lerp(*pCursor, wet, m_params.mix);
				++pCursor;
			}
			m_ringBuffer.advanceCursor();

			m_lfoTimeRate += lfoSpeed;
			if (m_lfoTimeRate > 1.0f)
			{
				m_lfoTimeRate = DecimalPart(m_lfoTimeRate);
			}
		}
	}

	bool FlangerDSP::getBypass() const
	{
		return m_bypass;
	}

	void FlangerDSP::setBypass(bool b)
	{
		m_bypass = b;
	}

	void FlangerDSP::setParams(const FlangerParams& params)
	{
		m_params = params;
	}

}
