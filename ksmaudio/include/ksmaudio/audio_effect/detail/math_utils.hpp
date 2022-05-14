#pragma once
#include <cmath>

namespace ksmaudio::AudioEffect
{

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
