#include "ksmaudio/audio_effect/dsp/retrigger_echo_dsp.hpp"

namespace ksmaudio::AudioEffect
{
    RetriggerEchoDSP::RetriggerEchoDSP(const DSPCommonInfo& info)
        : m_info(info)
        , m_linearBuffer(
            static_cast<std::size_t>(info.sampleRate) * 10 * info.numChannels, // 10 seconds
            info.numChannels)
    {
    }

    void RetriggerEchoDSP::process(float* pData, std::size_t dataSize, bool bypass, const RetriggerEchoDSPParams& params)
    {
        assert(dataSize % m_info.numChannels == 0);

        if (params.secUntilTrigger >= 0.0) // Negative value is ignored
        {
            m_framesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * static_cast<float>(m_info.sampleRate));
        }

        // updateTriggerによるトリガ更新
        // ("update_trigger"を"off>on"や"off-on"などにした場合のノーツ判定による更新)
        if (params.updateTrigger)
        {
            m_linearBuffer.resetReadWriteCursors();
        }

        const bool active = !bypass && params.mix > 0.0f;
        const std::size_t frameSize = dataSize / m_info.numChannels;
        const std::size_t numLoopFrames = static_cast<std::size_t>(params.waveLength * static_cast<float>(m_info.sampleRate));
        const std::size_t numNonZeroFrames = static_cast<std::size_t>(static_cast<float>(numLoopFrames) * params.rate);
        if (0 <= m_framesUntilTrigger && std::cmp_less(m_framesUntilTrigger, frameSize))
        {
            const std::size_t formerSize = static_cast<std::size_t>(m_framesUntilTrigger) * m_info.numChannels;
            m_linearBuffer.write(pData, formerSize);
            if (active)
            {
                m_linearBuffer.read(pData, formerSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix);
            }

            // framesUntilTriggerによるトリガ更新
            // ("update_period"や、"update_trigger"を"param_change"で"on"に変更した場合の更新)
            m_linearBuffer.resetReadWriteCursors();
            m_framesUntilTrigger = -1;

            const std::size_t latterSize = dataSize - formerSize;
            m_linearBuffer.write(pData + formerSize, latterSize);
            if (active)
            {
                m_linearBuffer.read(pData + formerSize, latterSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix);
            }
            else
            {
                m_linearBuffer.resetFadeOutScale();
            }
        }
        else
        {
            m_linearBuffer.write(pData, dataSize);
            if (active)
            {
                m_linearBuffer.read(pData, dataSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix);
            }
            else
            {
                m_linearBuffer.resetFadeOutScale();
            }
        }
    }
}
