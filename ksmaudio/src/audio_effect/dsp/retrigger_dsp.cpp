#include "ksmaudio/audio_effect/dsp/retrigger_dsp.hpp"

namespace ksmaudio::AudioEffect
{
    RetriggerDSP::RetriggerDSP(const DSPCommonInfo& info)
        : m_info(info)
        , m_linearBuffer(
            static_cast<std::size_t>(info.sampleRate) * 10 * info.numChannels, // 10 seconds
            info.numChannels)
    {
    }

    void RetriggerDSP::process(float* pData, std::size_t dataSize, bool bypass, RetriggerDSPParams& params)
    {
        assert(dataSize % m_info.numChannels == 0);

        if (params.secUntilTrigger >= 0.0) // Negative value is ignored
        {
            m_framesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * m_info.sampleRate);
        }

        const std::size_t frameSize = dataSize / m_info.numChannels;
        const std::size_t numLoopFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        const std::size_t numNonZeroFrames = static_cast<std::size_t>(numLoopFrames * params.rate);
        if (0 <= m_framesUntilTrigger && std::cmp_less(m_framesUntilTrigger, frameSize))
        {
            const std::size_t formerSize = static_cast<std::size_t>(m_framesUntilTrigger) * m_info.numChannels;
            m_linearBuffer.write(pData, formerSize);
            if (params.mix > 0.0f)
            {
                m_linearBuffer.read(pData, formerSize, numLoopFrames, numNonZeroFrames, params.mix);
            }

            // Update trigger
            m_linearBuffer.resetReadWriteCursors();
            m_framesUntilTrigger = -1;

            const std::size_t latterSize = dataSize - formerSize;
            m_linearBuffer.write(pData + formerSize, latterSize);
            if (params.mix > 0.0f)
            {
                m_linearBuffer.read(pData + formerSize, latterSize, numLoopFrames, numNonZeroFrames, params.mix);
            }
        }
        else
        {
            m_linearBuffer.write(pData, dataSize);
            if (params.mix > 0.0f)
            {
                m_linearBuffer.read(pData, dataSize, numLoopFrames, numNonZeroFrames, params.mix);
            }
        }
    }
}
