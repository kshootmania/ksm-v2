#pragma once
#include <algorithm>
#include "ring_buffer.hpp"

namespace ksmaudio::AudioEffect::detail
{
    class TimeModulator
    {
    private:
        RingBuffer<float> m_ringBuffer;
        float m_delaySample = 0.0f;
        const float m_maxDelayFrames;

    public:
        TimeModulator(std::size_t size, std::size_t numChannels)
            : m_ringBuffer(size, numChannels)
            , m_maxDelayFrames(static_cast<float>(m_ringBuffer.numFrames()))
        {
        }

        void update(float playSpeed)
        {
            // 再生速度をもとにディレイタイムを更新
            if (playSpeed != 1.0f)
            {
                m_delaySample = std::clamp(m_delaySample + 1.0f - playSpeed, 0.0f, m_maxDelayFrames);
            }
        }

        // 1チャンネルについて値を読み込み/書き込み
        // (updateまたはresetDelayTimeを呼んだ後で使用すること)
        float readWrite(float value, std::size_t channel)
        {
            // バッファにサンプルを記録
            m_ringBuffer.write(value, channel);

            // ディレイサンプルをバッファから取得
            float wet;
            if (m_delaySample > 0.0f)
            {
                wet = m_ringBuffer.lerpedDelay(m_delaySample, channel);
            }
            else
            {
                wet = value;
            }

            return wet;
        }

        void advanceCursor()
        {
            m_ringBuffer.advanceCursor();
        }

        void advanceCursor(std::size_t frameCount)
        {
            m_ringBuffer.advanceCursor(frameCount);
        }

        // 全チャンネルについてまとめて値を書き込み
        // (updateまたはresetDelayTimeを呼んだ後で使用すること)
        void writeAndAdvanceCursor(const float* pData, std::size_t size)
        {
            assert(size % m_ringBuffer.numChannels() == 0U);
            m_ringBuffer.write(pData, size);
            m_ringBuffer.advanceCursor(size / m_ringBuffer.numChannels());
        }

        // ディレイタイムをリセットして0にする
        // (再生速度をLFOで揺らす場合に、周期ごとにこれを呼ぶと浮動小数点数の計算誤差の蓄積を防止できる)
        void resetDelayTime()
        {
            m_delaySample = 0.0f;
        }
    };
}
