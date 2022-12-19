#pragma once
#include <algorithm>
#include <cmath>
#include <cassert>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct HighPassFilterDSPParams
	{
		float v = 0.0f;
		float q = 5.0f;
		float mix = 1.0f;
	};

	struct HighPassFilterParams
	{
		// TODO: freq、freq_maxの値を変更可能にする
		Param v = DefineParam(Type::kRate, "0%-100%");
		//Param freq = DefineParam(Type::kFreq, "100Hz");
		//Param freqMax = DefineParam(Type::kFreq, "4600Hz");
		Param q = DefineParam(Type::kFloat, "5.0");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kV, &v },
			//{ ParamID::kFreq, &freq },
			//{ ParamID::kFreqMax, &freqMax },
			{ ParamID::kQ, &q },
			{ ParamID::kMix, &mix },
		};

		HighPassFilterDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.v = GetValue(v, status, isOn),
				.q = GetValue(q, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
