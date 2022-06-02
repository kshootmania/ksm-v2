#pragma once

namespace MusicGame
{
	template <typename T>
	class Timeline
	{
	private:
		const kson::ByPulse<T> m_map;
		kson::ByPulse<T>::const_iterator m_cursorItr;
		kson::ByPulse<T>::const_iterator m_nextCursorItr;
		kson::Pulse m_time = kPastPulse;

	public:
		explicit Timeline(const kson::ByPulse<T>& map)
			: m_map(map)
			, m_cursorItr(m_map.cbegin())
			, m_nextCursorItr(m_map.empty() ? m_map.cend() : std::next(m_map.cbegin()))
		{
		}

		bool update(kson::Pulse time)
		{
			if (time < m_time) [[unlikely]]
			{
				return false;
			}

			m_time = time;

			if (m_nextCursorItr == m_map.cend())
			{
				return false;
			}

			if (m_nextCursorItr->first > m_time)
			{
				return false;
			}

			const auto itr = detail::CurrentAt(m_map, m_time);
			m_cursorItr = itr;
			m_nextCursorItr = itr == m_map.cend() ? itr : std::next(itr);

			return false;
		}

		bool hasValue() const
		{
			return m_cursorItr != m_map.cend() && m_cursorItr->first <= m_time;
		}

		const T& value() const
		{
			assert(hasValue());
			return m_cursorItr->second;
		}
	};
}
