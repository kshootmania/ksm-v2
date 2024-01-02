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

        const std::size_t frameSize = dataSize / m_info.numChannels;
        const std::size_t numLoopFrames = static_cast<std::size_t>(params.waveLength * m_info.sampleRate);
        const std::size_t numNonZeroFrames = static_cast<std::size_t>(numLoopFrames * params.rate);
        if (0 <= m_framesUntilTrigger && std::cmp_less(m_framesUntilTrigger, frameSize)) // m_framesUntilTrigger < frameSize
        {
            // 今回の処理フレーム中にトリガ更新タイミングが含まれている場合、トリガ更新の前後2つに分けて処理

            // トリガ更新より前
            const std::size_t formerSize = static_cast<std::size_t>(m_framesUntilTrigger) * m_info.numChannels;
            m_linearBuffer.write(pData, formerSize);
            m_linearBuffer.read(pData, formerSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix, bypass);

            // framesUntilTriggerによるトリガ更新
            // ("update_period"や、"update_trigger"を"param_change"で"on"に変更した場合の更新)
            m_linearBuffer.resetReadWriteCursors();
            m_framesUntilTrigger = -1;

            // トリガ更新より後ろ
            const std::size_t latterSize = dataSize - formerSize;
            m_linearBuffer.write(pData + formerSize, latterSize);
            m_linearBuffer.read(pData + formerSize, latterSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix, bypass);
        }
        else
        {
            // 今回の処理フレーム中にトリガ更新タイミングが含まれていない場合、一度に処理

            m_linearBuffer.write(pData, dataSize);
            m_linearBuffer.read(pData, dataSize, numLoopFrames, numNonZeroFrames, params.fadesOut, params.feedbackLevel, params.mix, bypass);

            // 次回トリガ更新タイミングまでの残り時間を減らす
            if (std::cmp_greater_equal(m_framesUntilTrigger, frameSize)) // m_framesUntilTrigger >= frameSize
            {
                m_framesUntilTrigger -= frameSize;
            }
        }
    }

    void RetriggerEchoDSP::updateParams(const RetriggerEchoDSPParams& params)
    {
        // トリガ更新までのフレーム数を計算
        if (params.secUntilTrigger >= 0.0f) // 負の値は無視
        {
            const std::ptrdiff_t newFramesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * m_info.sampleRate);

            // 前回より小さい場合のみ反映(トリガ発生寸前に次の時間が入ることでトリガが抜ける現象を回避するため)
            if (m_framesUntilTrigger < 0 || m_framesUntilTrigger > newFramesUntilTrigger)
            {
                m_framesUntilTrigger = newFramesUntilTrigger;
            }
        }

        // updateTriggerによるトリガ更新
        // ("update_trigger"を"off>on"や"off-on"などにした場合のノーツ判定による更新)
        if (params.updateTrigger)
        {
            m_linearBuffer.resetReadWriteCursors();
        }
    }
}
