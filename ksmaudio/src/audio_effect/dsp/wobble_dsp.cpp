#include "ksmaudio/audio_effect/dsp/wobble_dsp.hpp"

namespace ksmaudio::AudioEffect
{
    namespace
    {
        // sin(pi / 2.25)
        constexpr float kSinPi_2_25 = 0.9848077893f;
        
        // Returns 0-1
        float LFOValue(std::size_t framesSincePrevTrigger, std::size_t numPeriodFrames)
        {
            float value = static_cast<float>(framesSincePrevTrigger % numPeriodFrames) / numPeriodFrames;
            value = ((value > 0.5f) ? (1.0f - value) : value) * 2;
            value = std::sin(value * std::numbers::pi_v<float> / 2);
            value = std::sin(value * std::numbers::pi_v<float> / 2.25f) / kSinPi_2_25;
            return value;
        }

        float WobbleFreq(std::size_t framesSincePrevTrigger, std::size_t numPeriodFrames, float loFreq, float hiFreq)
        {
            const float lfoValue = LFOValue(framesSincePrevTrigger, numPeriodFrames);
            return std::lerp(hiFreq, loFreq, lfoValue);
        }
    }

    WobbleDSP::WobbleDSP(const DSPCommonInfo& info)
        : m_info(info)
    {
    }

    void WobbleDSP::process(float* pData, std::size_t dataSize, bool bypass, const WobbleDSPParams& params)
    {
        assert(dataSize % m_info.numChannels == 0);

        if (params.secUntilTrigger >= 0.0) // Negative value is ignored
        {
            m_framesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * m_info.sampleRate);
        }

        const std::size_t frameSize = dataSize / m_info.numChannels;
        const std::size_t numPeriodFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        
        if (bypass || params.mix == 0.0f)
        {
            if (m_framesUntilTrigger < 0)
            {
                m_framesSincePrevTrigger += frameSize;
            }
            else if (m_framesUntilTrigger > frameSize)
            {
                m_framesSincePrevTrigger += frameSize;
                m_framesUntilTrigger -= frameSize;
            }
            else
            {
                m_framesSincePrevTrigger = frameSize - m_framesUntilTrigger;
                m_framesUntilTrigger = -1;
            }

            // Process the last frame to avoid noise at the beginning of the wobble audio effect
            if (frameSize > 0U) [[likely]]
            {
                // Note: Here, framesSincePrevTrigger is one frame different, but it doesn't matter much.
                const float freq = WobbleFreq(m_framesSincePrevTrigger, numPeriodFrames, params.loFreq, params.hiFreq);
                for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
                {
                    m_lowPassFilters[ch].process(pData[(frameSize - 1U) * m_info.numChannels + ch]);
                }
            }

            return;
        }

        // Wobble processing main
        const float fSampleRate = static_cast<float>(m_info.sampleRate);
        for (std::size_t i = 0U; i < frameSize; ++i)
        {
            const float freq = WobbleFreq(m_framesSincePrevTrigger, numPeriodFrames, params.loFreq, params.hiFreq);
            for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
            {
                m_lowPassFilters[ch].setLowPassFilter(freq, params.q, fSampleRate);
                *pData = m_lowPassFilters[ch].process(*pData);
                ++pData;
            }
            ++m_framesSincePrevTrigger;
        }
    }
}
