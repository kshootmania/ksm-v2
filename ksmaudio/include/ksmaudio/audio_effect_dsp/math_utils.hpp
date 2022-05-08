#pragma once
#include <cmath>

namespace ksmaudio
{

    template <typename T>
    constexpr T Lerp(T a, T b, T rate)
    {
        return a + (b - a) * rate;
    }

    template <typename T>
    constexpr T DecimalPart(T a)
    {
        return a - std::floor(a);
    }

	template <typename T>
	T Triangle(T timeRate)
	{
		if (timeRate < T{ 0.5 })
		{
			return timeRate * 2;
		}
		else
		{
			return T{ 2 } - timeRate * 2;
		}
	}

}
