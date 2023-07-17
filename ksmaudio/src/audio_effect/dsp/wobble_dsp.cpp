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
            if (numPeriodFrames == 0U)
            {
                return 0.0;
            }

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

        const std::size_t frameSize = dataSize / m_info.numChannels;
        const std::size_t numPeriodFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        const float fSampleRate = static_cast<float>(m_info.sampleRate);
        
        if (bypass || params.mix == 0.0f)
        {
            m_triggerHandler.advanceBatch(frameSize);

            // Process frames even if bypassed to avoid noise at the beginning of effects
            if (numPeriodFrames > 0U)
            {
                // Here, a fixed frequency is used to reduce computational costs
                const float freq = WobbleFreq(m_triggerHandler.framesSincePrevTrigger(), numPeriodFrames, params.freq1, params.freq2);
                for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
                {
                    m_lowPassFilters[ch].setLowPassFilter(freq, params.q, fSampleRate);
                }
                for (std::size_t i = 0U; i < frameSize; ++i)
                {
                    for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
                    {
                        m_lowPassFilters[ch].process(*pData);
                        ++pData;
                    }
                }
            }

            return;
        }

        // Wobble processing main
        for (std::size_t i = 0U; i < frameSize; ++i)
        {
            const float freq = WobbleFreq(m_triggerHandler.framesSincePrevTrigger(), numPeriodFrames, params.freq1, params.freq2);
            for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
            {
                m_lowPassFilters[ch].setLowPassFilter(freq, params.q, fSampleRate);
                *pData = m_lowPassFilters[ch].process(*pData);
                ++pData;
            }
            m_triggerHandler.advance();
        }
    }

    void WobbleDSP::updateParams(const WobbleDSPParams& params)
    {
        m_triggerHandler.setFramesUntilTrigger(params.secUntilTrigger, m_info.sampleRate);
    }
}
