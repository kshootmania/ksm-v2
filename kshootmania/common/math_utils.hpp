#pragma once
#include <concepts>
#include "kson/common/common.hpp"

namespace MathUtils
{
	// 必ず正の値を返す剰余(浮動小数点数版)
	template <typename T>
	T WrappedFmod(T a, T b) requires std::is_floating_point_v<T>
	{
		assert(b > std::numeric_limits<T>::epsilon());
		if (b <= std::numeric_limits<T>::epsilon())
		{
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
		assert(b != T{ 0 });
		if (b == T{ 0 })
		{
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
}
