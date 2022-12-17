#pragma once
#include <cstddef>

namespace ksmaudio::AudioEffect::detail
{
    class DSPSimpleTriggerHandler
    {
    private:
        std::size_t m_framesSincePrevTrigger = 0U;
        std::ptrdiff_t m_framesUntilTrigger = -1;

    public:
        DSPSimpleTriggerHandler() = default;

        const std::size_t& framesSincePrevTrigger() const
        {
            return m_framesSincePrevTrigger;
        }

        void advance()
        {
            ++m_framesSincePrevTrigger;
            if (m_framesUntilTrigger < 0)
            {
                return;
            }
            if (--m_framesUntilTrigger == 0)
            {
                m_framesSincePrevTrigger = 0U;
            }
        }

        void advanceBatch(std::size_t frames)
        {
            if (m_framesUntilTrigger < 0)
            {
                m_framesSincePrevTrigger += frames;
            }
            else if (m_framesUntilTrigger > static_cast<std::ptrdiff_t>(frames))
            {
                m_framesSincePrevTrigger += frames;
                m_framesUntilTrigger -= frames;
            }
            else
            {
                m_framesSincePrevTrigger = frames - m_framesUntilTrigger;
                m_framesUntilTrigger = -1;
            }
        }

        void setFramesUntilTrigger(float secUntilTrigger, std::size_t sampleRate)
        {
            if (secUntilTrigger >= 0.0f) // Negative value is ignored
            {
                const std::ptrdiff_t newFramesUntilTrigger = static_cast<std::ptrdiff_t>(secUntilTrigger * static_cast<float>(sampleRate));

                // ‘O‰ñ‚æ‚è¬‚³‚¢ê‡‚Ì‚İ”½‰f(ƒgƒŠƒK”­¶¡‘O‚ÉŸ‚ÌŠÔ‚ª“ü‚é‚±‚Æ‚ÅƒgƒŠƒK‚ª”²‚¯‚éŒ»Û‚ğ‰ñ”ğ‚·‚é‚½‚ß)
                if (m_framesUntilTrigger < 0 || m_framesUntilTrigger > newFramesUntilTrigger)
                {
                    m_framesUntilTrigger = newFramesUntilTrigger;
                    if (m_framesUntilTrigger == 0)
                    {
                        m_framesSincePrevTrigger = 0U;
                    }
                }
            }
        }
    };
}
