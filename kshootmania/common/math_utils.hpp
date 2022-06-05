#pragma once
#include <concepts>
#include "kson/common/common.hpp"

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

	template <typename T>
	constexpr kson::Ms SecToMs(T sec)
	{
		return static_cast<kson::Ms>(sec * 1000);
	}

	template <typename T = double>
	constexpr T MsToSec(kson::Ms ms)
	{
		return static_cast<T>(ms / 1000);
	}

	int32 NumDigits(int32 number);
}
