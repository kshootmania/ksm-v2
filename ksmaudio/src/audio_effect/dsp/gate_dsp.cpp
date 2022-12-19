#include "ksmaudio/audio_effect/dsp/gate_dsp.hpp"

namespace ksmaudio::AudioEffect
{
    namespace
    {
        constexpr std::size_t kDeclickFrames = 12U;

        constexpr float Scale(std::size_t framesSincePrevTrigger, std::size_t numPeriodFrames, std::size_t numNonZeroFrames)
        {
            if (numPeriodFrames == 0U)
            {
                return 1.0f;
            }

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

    void GateDSP::process(float* pData, std::size_t dataSize, bool bypass, const GateDSPParams& params, bool isParamUpdated)
    {
        assert(dataSize % m_info.numChannels == 0);

        if (isParamUpdated) // secUntilTriggerの値はパラメータ更新後の初回実行時のみ有効
        {
            m_triggerHandler.setFramesUntilTrigger(params.secUntilTrigger, m_info.sampleRate);
        }

        const std::size_t frameSize = dataSize / m_info.numChannels;
        
        if (bypass || params.mix == 0.0f)
        {
            m_triggerHandler.advanceBatch(frameSize);
            return;
        }

        const std::size_t numPeriodFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        const std::size_t numNonZeroFrames = static_cast<std::size_t>(numPeriodFrames * params.rate);

        for (std::size_t i = 0U; i < frameSize; ++i)
        {
            const float scale = std::lerp(1.0f, Scale(m_triggerHandler.framesSincePrevTrigger(), numPeriodFrames, numNonZeroFrames), params.mix);
            for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
            {
                *pData *= scale;
                ++pData;
            }
            m_triggerHandler.advance();
        }
    }
}
