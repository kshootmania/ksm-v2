#pragma once
#include <concepts>
#include "kson/Common/Common.hpp"

namespace MathUtils
{
	// 必ず正の値を返す剰余(浮動小数点数版)
	template <typename T>
	[[nodiscard]]
	T WrappedFmod(T a, T b) noexcept requires std::is_floating_point_v<T>
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
	[[nodiscard]]
	T WrappedMod(T a, T b) noexcept requires std::is_integral_v<T>
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

	[[nodiscard]]
	int32 NumDigits(int32 number) noexcept;

	template <typename T = int32, typename U>
	[[nodiscard]]
	T RoundToInt(U value) noexcept requires std::is_floating_point_v<U>
	{
		return static_cast<T>(value + U{ 0.5 });
	}

	[[nodiscard]]
	inline bool AlmostEquals(double a, double b) noexcept
	{
		return kson::AlmostEquals(a, b);
	}

	template <typename T>
	[[nodiscard]]
	T LinearDamp(T current, T target, T maxSpeed, double dt) noexcept requires std::is_arithmetic_v<T>
	{
		const T absDiffMax = Max(static_cast<T>(maxSpeed * dt), T{ 0 });
		return current + std::clamp(target - current, -absDiffMax, absDiffMax);
	}
}
