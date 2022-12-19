#include "ksmaudio/audio_effect/detail/wave_length_utils.hpp"
#include <array>
#include <algorithm>
#include <cmath>

namespace ksmaudio::AudioEffect::detail::WaveLengthUtils
{
	namespace
	{
		constexpr std::array<int, 14> kQuantizationArray = {
			1,
			2,
			4,
			6,
			8,
			12,
			16,
			24,
			32,
			48,
			64,
			96,
			128,
			192,
		};

		constexpr int kQuantizationArraySize = static_cast<int>(kQuantizationArray.size());

		constexpr float kAlmostOne = 0.9999f;
	}

	int DenominatorToQuantizationArrayIdx(int d)
	{
		for (int i = 0; i < kQuantizationArraySize - 1; ++i)
		{
			if (kQuantizationArray[i] <= d && d < kQuantizationArray[i + 1])
			{
				return i;
			}
		}
		if (d >= kQuantizationArray.back())
		{
			return kQuantizationArraySize - 1;
		}
		return 0;
	}

	float Interpolate(float a, float b, float rate)
	{
		// Do not quantize if two values are the same
		// (This is an intentional difference from the HSP version. This unifies the slightly different behavior
		//  between FX and laser in the HSP version.)
		if (a == b)
		{
			return a;
		}

		// Interpolate two values in the fixed array
		const float aIdx = static_cast<float>(DenominatorToQuantizationArrayIdx(static_cast<int>(1 / a)));
		const float bIdx = static_cast<float>(DenominatorToQuantizationArrayIdx(static_cast<int>(1 / b)) + 1);
		const float lerpedIdx = std::lerp(aIdx, bIdx, std::min(rate, kAlmostOne));
		const int lerpedIdxInt = std::clamp(static_cast<int>(lerpedIdx), 0, kQuantizationArraySize - 1);
		return 1.0f / kQuantizationArray[lerpedIdxInt];
	}
}
