#include "ksmaudio/audio_effect/dsp/phaser_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	namespace
	{
		constexpr float kHiCutFilterQ = 1.5f;
		constexpr float kHiCutFilterGain = -8.0f;

		std::array<std::array<float, 2>, PhaserDSP::kMaxNumAllPassFilters> MakeZeroWetArray()
		{
			std::array<std::array<float, 2>, PhaserDSP::kMaxNumAllPassFilters> zeroWetArray;
			zeroWetArray.fill({ 0.0f, 0.0f });
			return zeroWetArray;
		}
	}

	PhaserDSP::PhaserDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_prevWetArrayForFeedback(MakeZeroWetArray())
	{
	}

	void PhaserDSP::process(float* pData, std::size_t dataSize, bool bypass, const PhaserDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		assert(dataSize % m_info.numChannels == 0);
		const std::size_t numFrames = dataSize / m_info.numChannels;
		const float periodSamples = params.period * m_info.sampleRate;
		const float lfoSpeed = periodSamples == 0.0f ? 0.0f : (1.0f / periodSamples);
		const float log10Freq1 = detail::Log10Freq(params.freq1);
		const float log10Freq2 = detail::Log10Freq(params.freq2);
		const float centerFreq = detail::InterpolateFreqInLog10ScaleWithPrecalculatedLog10(0.5f, log10Freq1, log10Freq2);
		for (std::size_t channel = 0; channel < m_info.numChannels; ++channel)
		{
			m_hiCutFilters[channel].setHighShelfFilter(centerFreq, kHiCutFilterQ, kHiCutFilterGain, m_info.sampleRateFloat);
		}

		const std::size_t stage = params.mix > 0.0f ? params.stage : 0U;
		if (stage > 0U)
		{
			for (std::size_t i = 0; i < numFrames; ++i)
			{
				std::array<std::array<float, 2>, kMaxNumAllPassFilters> wetArray = m_prevWetArrayForFeedback;
				for (std::size_t channel = 0; channel < m_info.numChannels; ++channel)
				{
					const float lfoValue = detail::TriangleWithStereoWidth(m_lfoTimeRate, channel, params.stereoWidth);
					const float freq = detail::InterpolateFreqInLog10ScaleWithPrecalculatedLog10(lfoValue, log10Freq1, log10Freq2);
					for (std::size_t s = 0; s < kMaxNumAllPassFilters; ++s)
					{
						const float input = s == 0U ? (*pData + m_prevWetArrayForFeedback[kMaxNumAllPassFilters - 1U][channel] * params.feedback) : wetArray[s - 1U][channel];
						if (s < stage)
						{
							m_allPassFilters[s][channel].setAllPassFilter(freq, params.q, m_info.sampleRateFloat);
							wetArray[s][channel] = m_allPassFilters[s][channel].process(input);
						}
						else
						{
							wetArray[s][channel] = input;
						}
					}

					const float wetFiltered = m_hiCutFilters[channel].process(wetArray[kMaxNumAllPassFilters - 1U][channel]);
					*pData = std::lerp(*pData, wetFiltered, params.mix / 2);
					++pData;
				}

				m_lfoTimeRate += lfoSpeed;
				if (m_lfoTimeRate > 1.0f)
				{
					m_lfoTimeRate = detail::DecimalPart(m_lfoTimeRate);
				}
				m_prevWetArrayForFeedback = wetArray;
			}
		}
		else
		{
			// チャンネル数は1か2のみ
			// (それ以外の場合はisUnsupportedがtrueになるためここに来ない)
			assert(m_info.numChannels == 1U || m_info.numChannels == 2U);

			m_lfoTimeRate = detail::DecimalPart(m_lfoTimeRate + lfoSpeed * numFrames);

			// 末尾のサンプルの値を反映
			if (m_info.numChannels == 1U)
			{
				m_prevWetArrayForFeedback.fill({ pData[dataSize - 1], pData[dataSize - 1] });
			}
			else
			{
				m_prevWetArrayForFeedback.fill({ pData[dataSize - 2], pData[dataSize - 1] });
			}
		}
	}

	void PhaserDSP::updateParams(const PhaserDSPParams& params)
	{
		// 特に何もしない
	}
}
