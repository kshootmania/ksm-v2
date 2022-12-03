#pragma once
#include <set>

namespace ksmaudio::AudioEffect::detail
{
    class UpdateTriggerTimeline
    {
	private:
		const std::set<float> m_updateTriggerTiming;
		std::set<float>::const_iterator m_updateTriggerTimingCursor;
		float m_secUntilTrigger = -1.0f;

	public:
		explicit UpdateTriggerTimeline(const std::set<float>& updateTriggerTiming)
			: m_updateTriggerTiming(updateTriggerTiming)
			, m_updateTriggerTimingCursor(m_updateTriggerTiming.begin())
		{
		}

		void update(float currentTimeSec)
		{
			// カーソルを現在時間まで進める
			while (m_updateTriggerTimingCursor != m_updateTriggerTiming.end() && *m_updateTriggerTimingCursor < currentTimeSec)
			{
				++m_updateTriggerTimingCursor;
			}

			if (m_updateTriggerTimingCursor == m_updateTriggerTiming.end())
			{
				// 負の値はDSP側で無視される
				m_secUntilTrigger = -1.0f;
				return;
			}

			m_secUntilTrigger = *m_updateTriggerTimingCursor - currentTimeSec;
		}

		float secUntilTrigger() const
		{
			return m_secUntilTrigger;
		}
    };
}
