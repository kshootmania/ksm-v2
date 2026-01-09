#include "ksmaudio/AudioEffect/DSP/FlangerDSP.hpp"

namespace ksmaudio::AudioEffect
{
	FlangerDSP::FlangerDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_ringBuffer(
			static_cast<std::size_t>(info.sampleRate) * 3 * info.numChannels, // 3 seconds
			info.numChannels)
	{
		for (auto& filter : m_lowShelfFilters)
		{
			filter.setLowShelfFilter(250.0f, 0.5f, -20.0f, static_cast<float>(info.sampleRate));
		}
	}

	void FlangerDSP::process(float* pData, std::size_t dataSize, bool bypass, const FlangerDSPParams& params)
	{
		if (m_info.isUnsupported || dataSize > m_ringBuffer.size())
		{
			return;
		}

		assert(dataSize % m_info.numChannels == 0);
		const std::size_t numFrames = dataSize / m_info.numChannels;
		if (bypass || params.mix == 0.0f)
		{
			m_ringBuffer.write(pData, dataSize);
			m_ringBuffer.advanceCursor(numFrames);
			return;
		}

		const float periodSamples = params.period * m_info.sampleRate;
		const float lfoSpeed = periodSamples == 0.0f ? 0.0f : (1.0f / periodSamples);
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			for (std::size_t channel = 0; channel < m_info.numChannels; ++channel)
			{
				const float lfoValue = detail::TriangleWithStereoWidth(m_lfoTimeRate, channel, params.stereoWidth);
				const float delayFrames = (params.delay + lfoValue * params.depth) * m_info.sampleRateScale;
				const float delayed = m_ringBuffer.lerpedDelay(delayFrames, channel);
				const float gain = std::lerp(1.0f, params.vol, params.mix);
				m_ringBuffer.write(m_lowShelfFilters[channel].process((*pData + delayed * params.feedback) * gain), channel);
				*pData = (*pData + delayed * params.mix) * gain;
				++pData;
			}
			m_ringBuffer.advanceCursor();

			m_lfoTimeRate += lfoSpeed;
			if (m_lfoTimeRate > 1.0f)
			{
				m_lfoTimeRate = detail::DecimalPart(m_lfoTimeRate);
			}
		}
	}

	void FlangerDSP::updateParams(const FlangerDSPParams& params)
	{
		// 特に何もしない
	}
}
