#pragma once
#include <concepts>
#include "ksh/common/common.hpp"

namespace MathUtils
{
	// Fmod function consistent for negative values
	template <typename T>
	T WrappedFmod(T a, T b) requires std::is_floating_point_v<T>
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

	template <typename T>
	T WrappedMod(T a, T b) requires std::is_integral_v<T>
	{
		const T mod = a % b;
		if (mod < T{ 0 })
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
