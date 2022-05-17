#pragma once
#include <vector>
#include <type_traits>
#include <iostream>
#include <memory>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include "math_utils.hpp"

namespace ksmaudio::AudioEffect::detail
{
    // TODO: No need to use class template
    template <typename T>
    class RingBuffer
    {
        static_assert(std::is_arithmetic_v<T>,
            "Value type of RingBuffer is required to be arithmetic");

    private:
        std::vector<T> m_buffer;

        std::size_t m_cursorFrame = 0U;

        const std::size_t m_numFrames;

        const std::size_t m_numChannels;

        std::size_t normalizeIdx(std::int64_t idx) const
        {
            const std::int64_t numFrames = static_cast<std::int64_t>(m_numFrames);
            const std::int64_t normalizedIdx = idx % numFrames;

            if (normalizedIdx < 0)
            {
                return static_cast<std::size_t>(normalizedIdx + numFrames);
            }
            else
            {
                return static_cast<std::size_t>(normalizedIdx);
            }
        }

        std::size_t delayCursor(std::size_t delayFrames) const
        {
            return (m_cursorFrame - (delayFrames + 1) % m_numFrames + m_numFrames) % m_numFrames;
        }

        void writeImpl(const T* pData, std::size_t size, std::size_t cursorFrame)
        {
            assert(size % m_numChannels == 0);

            const std::size_t frameSize = size / m_numChannels;
            if (cursorFrame + frameSize < m_numFrames)
            {
                std::memcpy(&m_buffer[cursorFrame * m_numChannels], pData, sizeof(float) * frameSize * m_numChannels);
            }
            else
            {
                const std::size_t firstFrameSize = m_numFrames - cursorFrame;
                std::memcpy(&m_buffer[cursorFrame * m_numChannels], pData, sizeof(float) * firstFrameSize * m_numChannels);
                const std::size_t secondFrameSize = frameSize - firstFrameSize;
                if (secondFrameSize > 0U)
                {
                    writeImpl(pData + firstFrameSize * m_numChannels, secondFrameSize * m_numChannels, 0U);
                }
            }
        }

    public:
        explicit RingBuffer(std::size_t size, std::size_t numChannels)
            : m_buffer(size, T{ 0 })
            , m_numFrames(size / numChannels)
            , m_numChannels(numChannels)
        {
            assert(m_numChannels > 0);
            assert(size % m_numChannels == 0);
            m_buffer.shrink_to_fit();
        }

        void write(const T* pData, std::size_t size)
        {
            writeImpl(pData, size, m_cursorFrame);
        }

        void write(float data, std::size_t channel)
        {
            m_buffer[m_cursorFrame * m_numChannels + channel] = data;
        }

        void advanceCursor()
        {
            if (++m_cursorFrame >= m_numFrames)
            {
                m_cursorFrame = 0U;
            }
        }

        void advanceCursor(std::size_t frameCount)
        {
            m_cursorFrame = (m_cursorFrame + frameCount) % m_numFrames;
        }

        T& delay(std::size_t delayFrames, std::size_t channel)
        {
            return m_buffer[delayCursor(delayFrames) * m_numChannels + channel];
        }

        const T& delay(std::size_t delayFrames, std::size_t channel) const
        {
            return m_buffer[delayCursor(delayFrames) * m_numChannels + channel];
        }

        void delay(std::size_t delayFrames, T* pDest)
        {
            std::memcpy(pDest, &m_buffer[delayCursor(delayFrames) * m_numChannels], sizeof(float) * m_numChannels);
        }

        template <typename U>
        T lerpedDelay(U floatDelayFrames, std::size_t channel) const
        {
            const std::size_t delayFrames = static_cast<std::size_t>(floatDelayFrames);
            return std::lerp(
                m_buffer[delayCursor(delayFrames) * m_numChannels + channel],
                m_buffer[delayCursor(delayFrames + 1) * m_numChannels + channel],
                DecimalPart(floatDelayFrames));
        }

        template <typename U>
        void lerpedDelay(U floatDelayFrames, T* pDest) const
        {
            const std::size_t delayFrames = static_cast<std::size_t>(floatDelayFrames);
            const std::size_t firstIdx = delayCursor(delayFrames) * m_numChannels;
            const std::size_t secondIdx = delayCursor(delayFrames + 1) * m_numChannels;
            const U lerpRate = DecimalPart(floatDelayFrames);
            for (std::size_t channel = 0; channel < m_numChannels; ++channel)
            {
                pDest[channel] = Lerp(m_buffer[firstIdx + channel], m_buffer[secondIdx + channel], lerpRate);
            }
        }

        std::size_t size() const
        {
            return m_buffer.size();
        }

        std::size_t numFrames() const
        {
            return m_numFrames;
        }

        std::size_t cursorFrame() const
        {
            return m_cursorFrame;
        }

        std::vector<T>& buffer()
        {
            return m_buffer;
        }

        const std::vector<T>& buffer() const
        {
            return m_buffer;
        }
    };
}
