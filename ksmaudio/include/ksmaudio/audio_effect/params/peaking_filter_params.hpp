#pragma once
#include <algorithm>
#include <cmath>
#include <cassert>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct PeakingFilterDSPParams
	{
		float freq = 1000.0f;
		float gain = 6.0f;
		float bandwidth = 1.2f;
		float mix = 1.0f;
	};

	namespace detail
	{
		// TODO: freq、freq_maxの値を変更可能にする
		inline float GetPeakingFilterFreqValue(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L974

			const float cosV = std::cos(v);
			const float cos2V = cosV * cosV;
			const float baseFreq = 10000.0f - 9950.0f * (cosV * 4 + cos2V * 5) / 9;

			constexpr float kSpeedUpThreshold = 0.8f;
			const float speedUpFreq = (v > kSpeedUpThreshold) ? 3000.0f * (v - kSpeedUpThreshold) / (1.0f - kSpeedUpThreshold) : 0.0f;

			return baseFreq + speedUpFreq;
		}

		// TODO: freq、freq_maxの値を加味する
		inline float GetPeakingFilterGainValue(float v, float gainRate)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L974

			const float baseGain = 14.0f * (std::min)(v, 0.15f) / 0.15f;
			const float midGain = 3.0f * (1.0f - std::abs(v - 0.35f) / 0.35f);
			constexpr float kHiGainCutThreshold = 0.8f;
			const float hiGainCut = (v > kHiGainCutThreshold) ? -3.5f * (v - kHiGainCutThreshold) / (1.0f - kHiGainCutThreshold) : 0.0f;
			return std::clamp(baseGain + midGain + hiGainCut, 0.0f, 18.0f) * gainRate / 0.5f;
		}
	}

	struct PeakingFilterParams
	{
		// TODO: freq、freq_maxの値を変更可能にする
		Param v = DefineParam(Type::kRate, "0%-100%");
		//Param freq = DefineParam(Type::kRate, "50Hz");
		//Param freqMax = DefineParam(Type::kRate, "13000Hz");
		Param gain = DefineParam(Type::kRate, "50%");
		Param bandwidth = DefineParam(Type::kFloat, "1.2");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kV, &v },
			//{ ParamID::kFreq, &freq },
			//{ ParamID::kFreqMax, &freqMax },
			{ ParamID::kGain, &gain },
			{ ParamID::kBandwidth, &bandwidth },
			{ ParamID::kMix, &mix },
		};

		PeakingFilterDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			const float vValue = GetValue(v, status, isOn);
			return {
				.freq = detail::GetPeakingFilterFreqValue(vValue),
				.gain = detail::GetPeakingFilterGainValue(vValue, GetValue(gain, status, isOn)),
				.bandwidth = GetValue(bandwidth, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
