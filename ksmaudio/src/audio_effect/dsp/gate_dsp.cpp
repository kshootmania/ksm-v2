#include "ksmaudio/audio_effect/dsp/gate_dsp.hpp"

namespace ksmaudio::AudioEffect
{
    namespace
    {
        constexpr std::size_t kDeclickFrames = 12U;

        constexpr float Scale(std::size_t framesSincePrevTrigger, std::size_t numPeriodFrames, std::size_t numNonZeroFrames)
        {
            const std::size_t framesSincePeriodStart = framesSincePrevTrigger % numPeriodFrames;
            if (framesSincePeriodStart < numNonZeroFrames)
            {
                return 1.0f;
            }
            else if (framesSincePeriodStart < numNonZeroFrames + kDeclickFrames && numNonZeroFrames > kDeclickFrames)
            {
                return static_cast<float>(kDeclickFrames - (framesSincePeriodStart - numNonZeroFrames)) / (kDeclickFrames + 1U);
            }
            else
            {
                return 0.0f;
            }
        }
    }

    GateDSP::GateDSP(const DSPCommonInfo& info)
        : m_info(info)
    {
    }

    void GateDSP::process(float* pData, std::size_t dataSize, bool bypass, const GateDSPParams& params)
    {
        assert(dataSize % m_info.numChannels == 0);

        if (params.secUntilTrigger >= 0.0) // Negative value is ignored
        {
            m_framesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * m_info.sampleRate);
        }

        const std::size_t frameSize = dataSize / m_info.numChannels;
        
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
            return;
        }

        const std::size_t numPeriodFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        const std::size_t numNonZeroFrames = static_cast<std::size_t>(numPeriodFrames * params.rate);

        for (std::size_t i = 0U; i < frameSize; ++i)
        {
            const float scale = std::lerp(1.0f, Scale(m_framesSincePrevTrigger, numPeriodFrames, numNonZeroFrames), params.mix);
            for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
            {
                *pData *= scale;
                ++pData;
            }
            ++m_framesSincePrevTrigger;
        }
    }
}
