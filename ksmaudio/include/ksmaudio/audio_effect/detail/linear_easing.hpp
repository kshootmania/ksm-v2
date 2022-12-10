#pragma once
#include <algorithm>

namespace ksmaudio::AudioEffect::detail
{
	template <typename T>
	class LinearEasing
	{
	private:
		const T m_speed;
		T m_value = T{ 0 };
		bool m_isFirst = true;

	public:
		explicit LinearEasing(T speed)
			: m_speed(speed)
		{
		}

		bool update(T targetValue, T deltaTime = 1.0f)
		{
			if (m_isFirst)
			{
				m_value = targetValue;
				m_isFirst = false;
				return true;
			}

			if (m_value < targetValue)
			{
				m_value = (std::min)(m_value + m_speed * deltaTime, targetValue);
				return true;
			}

			if (m_value > targetValue)
			{
				m_value = (std::max)(m_value - m_speed * deltaTime, targetValue);
				return true;
			}

			return false;
		}

		T value() const
		{
			return m_value;
		}
	};
}
