#pragma once
#include <algorithm>
#include <cmath>
#include <cassert>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct PeakingFilterDSPParams
	{
		float v = 0.0f;
		//float freq = 50.0f;
		//float freqMax = 13000.0f;
		float gainRate = 0.5f;
		float bandwidth = 1.2f;
		float mix = 1.0f;
	};

	struct PeakingFilterParams
	{
		// TODO: freq、freq_maxの値を変更可能にする
		Param v = DefineParam(Type::kRate, "0%-100%");
		//Param freq = DefineParam(Type::kRate, "50Hz");
		//Param freqMax = DefineParam(Type::kRate, "13000Hz");
		Param bandwidth = DefineParam(Type::kFloat, "1.2");
		Param gain = DefineParam(Type::kRate, "50%");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kV, &v },
			//{ ParamID::kFreq, &freq },
			//{ ParamID::kFreqMax, &freqMax },
			{ ParamID::kBandwidth, &bandwidth },
			{ ParamID::kGain, &gain },
			{ ParamID::kMix, &mix },
		};

		PeakingFilterDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.v = GetValue(v, status, isOn),
				.gainRate = GetValue(gain, status, isOn),
				.bandwidth = GetValue(bandwidth, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
