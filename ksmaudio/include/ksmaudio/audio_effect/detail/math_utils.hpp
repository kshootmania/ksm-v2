#pragma once
#include <algorithm>
#include <cmath>

namespace ksmaudio::AudioEffect::detail
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

	template <typename T>
	T TriangleWithStereoWidth(T timeRate, std::size_t channel, T stereoWidth)
	{
		if (channel == 0U)
		{
			return Triangle(timeRate);
		}
		else
		{
			return Triangle(DecimalPart(timeRate + stereoWidth / 2));
		}
	}

	template <typename T>
	T ClampFreq(T freq)
	{
		return std::clamp(freq, T{ 10 }, T{ 22050 });
	}

	template <typename T>
	T Log10Freq(T freq)
	{
		return std::log10(ClampFreq(freq));
	}

	template <typename T>
	T InterpolateFreqInLog10ScaleWithPrecalculatedLog10(T v, T log10Freq1, T log10Freq2)
	{
		const T log10Freq = std::lerp(log10Freq1, log10Freq2, v);
		return std::pow(T{ 10 }, log10Freq);
	}

	template <typename T>
	T InterpolateFreqInLog10Scale(T v, T freq1, T freq2)
	{
		if (freq1 == freq2)
		{
			return freq1;
		}

		const T log10Freq1 = Log10Freq(freq1);
		const T log10Freq2 = Log10Freq(freq2);
		return InterpolateFreqInLog10ScaleWithPrecalculatedLog10(v, log10Freq1, log10Freq2);
	}
}
