#pragma once
#include <algorithm>
#include <vector>
#include <type_traits>
#include <cassert>
#include <cstring>
#include <cstddef>
#include "math_utils.hpp"
#ifdef min
#undef min
#endif

namespace ksmaudio::AudioEffect::detail
{
    // Useful for audio recording
    template <typename T>
    class LinearBuffer
    {
        static_assert(std::is_arithmetic_v<T>,
            "Value type of LinearBuffer is required to be arithmetic");

    private:
        std::vector<T> m_buffer;

        std::size_t m_readCursorFrame = 0U;

        std::size_t m_writeCursorFrame = 0U;

        const std::size_t m_numFrames;

        const std::size_t m_numChannels;

    public:
        explicit LinearBuffer(std::size_t size, std::size_t numChannels)
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
            assert(size % m_numChannels == 0);

            const std::size_t frameSize = size / m_numChannels;
            const std::size_t numWriteFrames = std::min(frameSize, m_numFrames - m_writeCursorFrame);
            if (numWriteFrames == 0U)
            {
                return;
            }
            std::memcpy(&m_buffer[m_writeCursorFrame * m_numChannels], pData, sizeof(T) * numWriteFrames * m_numChannels);
            m_writeCursorFrame += numWriteFrames;
        }

        void read(T* pData, std::size_t size, std::size_t numLoopFrames, std::size_t numNonZeroFrames, float mix = 1.0f)
        {
            assert(size % m_numChannels == 0);

            if (numLoopFrames > m_numFrames) [[unlikely]]
            {
                return;
            }

            if (numNonZeroFrames > numLoopFrames) [[unlikely]]
            {
                numNonZeroFrames = numLoopFrames;
            }

            if (m_readCursorFrame >= numLoopFrames)
            {
                m_readCursorFrame = 0U;
            }

            const std::size_t frameSize = size / m_numChannels;
            for (std::size_t frameIdx = 0U; frameIdx < frameSize; ++frameIdx)
            {
                for (std::size_t ch = 0U; ch < m_numChannels; ++ch)
                {
                    if (m_readCursorFrame < numNonZeroFrames)
                    {
                        *pData = std::lerp(*pData, m_buffer[m_readCursorFrame * m_numChannels + ch], mix);
                    }
                    else
                    {
                        *pData = std::lerp(*pData, T{ 0 }, mix);
                    }
                    ++pData;
                }

                if (++m_readCursorFrame >= numLoopFrames)
                {
                    m_readCursorFrame = 0U;
                }
            }
        }

        void read(T* pData, std::size_t size, std::size_t numLoopFrames)
        {
            read(pData, size, numLoopFrames, numLoopFrames);
        }

        void read(T* pData, std::size_t size)
        {
            assert(size % m_numChannels == 0U);

            const std::size_t frameSize = size / m_numChannels;
            if (m_writeCursorFrame <= m_readCursorFrame)
            {
                std::memset(pData, 0, sizeof(T) * frameSize * m_numChannels); // HACK: This assumes IEEE 754
            }
            else if (m_writeCursorFrame - m_readCursorFrame < frameSize)
            {
                const std::size_t numReadFrames = m_numFrames - m_readCursorFrame;
                std::memcpy(pData, &m_buffer[m_readCursorFrame * m_numChannels], sizeof(T) * numReadFrames * m_numChannels);
                std::memset(pData + numReadFrames * m_numChannels, 0, sizeof(T) * (frameSize - numReadFrames) * m_numChannels); // HACK: This assumes IEEE 754
                m_readCursorFrame += numReadFrames;
            }
            else
            {
                std::memcpy(pData, &m_buffer[m_readCursorFrame * m_numChannels], sizeof(T) * frameSize * m_numChannels);
                m_readCursorFrame += frameSize;
            }
        }

        void resetReadCursor()
        {
            m_readCursorFrame = 0U;
        }

        void resetReadWriteCursors()
        {
            m_readCursorFrame = 0U;
            m_writeCursorFrame = 0U;
        }

        std::size_t size() const
        {
            return m_buffer.size();
        }

        std::size_t numFrames() const
        {
            return m_numFrames;
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
