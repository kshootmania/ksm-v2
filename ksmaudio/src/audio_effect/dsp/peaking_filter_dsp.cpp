#include "ksmaudio/audio_effect/dsp/peaking_filter_dsp.hpp"
#include <algorithm>

namespace ksmaudio::AudioEffect
{
	namespace
	{
		constexpr float kFreqSpeed = 5.0f;
	}

	PeakingFilterDSP::PeakingFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_freqEasing(kFreqSpeed)
	{
	}

	void PeakingFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}


		const float fSampleRate = static_cast<float>(m_info.sampleRate);

		const bool isBypassed = bypass || params.mix == 0.0f; // Ø‚è‘Ö‚¦Žž‚ÌƒmƒCƒY‰ñ”ð‚Ì‚½‚ß‚Ébypassó‘Ô‚Å‚àˆ—Ž©‘Ì‚Í‚·‚é
		const bool isSkipped = std::abs(params.v - m_prevV) > 0.2f || params.freq < 60.0f; // ƒmƒCƒY‰ñ”ð‚Ì‚½‚ßA’¼Šp“™‚Å’l‚ª”ò‚ñ‚¾uŠÔ‚âA’áŽü”g”‚É‘Î‚µ‚Ä‚Í“K—p‚µ‚È‚¢
		// TODO: isSkipped‚Í’¼Šp‚©‚çˆê’èŽžŠÔ‚©‚Ç‚¤‚©‚Å”»’è‚·‚é‚æ‚¤‚É‚·‚é

		assert(dataSize % m_info.numChannels == 0);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			const bool freqUpdated = m_freqEasing.update(params.freq);

			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				if (freqUpdated)
				{
					m_peakingFilters[ch].setPeakingFilter(m_freqEasing.value(), params.bandwidth, params.gain, fSampleRate);
				}

				const float wet = m_peakingFilters[ch].process(*pData);
				if (!isBypassed && !isSkipped)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}
		}

		m_prevV = params.v;
	}
}
