#pragma once
#include "ksh/common/common.hpp"

namespace MathUtils
{
	// Fmod function consistent for negative values
	template <typename T>
	T WrappedFmod(T a, T b)
	{
		const T mod = std::fmod(a, b);
		if (mod < T{ 0.0 })
		{
			return mod + Abs(b); // wrap the value to positive
		}
		else
		{
			return mod;
		}
	}

	ksh::Ms SecToMs(double sec);
}
