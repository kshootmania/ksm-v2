#pragma once
#include <concepts>
#include "kson/common/common.hpp"

namespace MathUtils
{
	// 必ず正の値を返す剰余(浮動小数点数版)
	template <typename T>
	T WrappedFmod(T a, T b) requires std::is_floating_point_v<T>
	{
		if (b <= std::numeric_limits<T>::epsilon())
		{
			assert(false && "WrappedFmod: zero division detected");
			return T{ 0.0 };
		}

		const T mod = std::fmod(a, b);
		if (mod < T{ 0.0 })
		{
			return mod + Abs(b); // fmodの結果が負の値になった場合は正の値へ
		}
		else
		{
			return mod;
		}
	}

	// 必ず正の値を返す剰余(整数版)
	template <typename T>
	T WrappedMod(T a, T b) requires std::is_integral_v<T>
	{
		if (b == T{ 0 })
		{
			assert(false && "WrappedMod: zero division detected");
			return T{ 0 };
		}

		const T mod = a % b;
		if (mod < T{ 0 })
		{
			return mod + Abs(b); // %の結果が負の値になった場合は正の値へ
		}
		else
		{
			return mod;
		}
	}

	int32 NumDigits(int32 number);

	template <typename T = int32, typename U>
	T RoundToInt(U value) requires std::is_floating_point_v<U>
	{
		return static_cast<T>(value + U{ 0.5 });
	}

	inline bool AlmostEquals(double a, double b)
	{
		return kson::AlmostEquals(a, b);
	}
}
