#pragma once
#include <algorithm>
#include <cmath>
#include <cassert>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct LowPassFilterDSPParams
	{
		float v = 0.0f;
		float freq = 8000.0f;
		float q = 5.0f;
		float mix = 1.0f;
	};

	namespace detail
	{
		// TODO: freq、freq_maxの値を変更可能にする
		inline float GetLowPassFilterFreqValue(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L957

			return 15000.0f - 14200.0f * std::sin(std::sin(v * 1.25f) / sin(1.25f) * 1.5f) / sin(1.5f);
		}
	}

	struct LowPassFilterParams
	{
		// TODO: freq、freq_maxの値を変更可能にする
		Param v = DefineParam(Type::kRate, "0%-100%");
		//Param freq = DefineParam(Type::kFreq, "15000Hz");
		//Param freqMax = DefineParam(Type::kFreq, "800Hz");
		Param q = DefineParam(Type::kFloat, "5.0");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kV, &v },
			//{ ParamID::kFreq, &freq },
			//{ ParamID::kFreqMax, &freqMax },
			{ ParamID::kQ, &q },
			{ ParamID::kMix, &mix },
		};

		LowPassFilterDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			const float vValue = GetValue(v, status, isOn);
			return {
				.v = vValue,
				.freq = detail::GetLowPassFilterFreqValue(vValue),
				.q = GetValue(q, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
