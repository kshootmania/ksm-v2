#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct WobbleDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: Negative value will be just ignored
		float waveLength = 0.0f;
		float loFreq = 500.0f;
		float hiFreq = 20000.0f;
		float q = 1.414f;
		float mix = 0.5f;
	};

	struct WobbleParams
	{
		Param waveLength = DefineParam(Type::kLength, "0");
		Param loFreq = DefineParam(Type::kFreq, "500Hz");
		Param hiFreq = DefineParam(Type::kFreq, "20000Hz");
		Param q = DefineParam(Type::kFloat, "1.414");
		Param mix = DefineParam(Type::kRate, "0%>50%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kWaveLength, &waveLength },
			{ ParamID::kLoFreq, &loFreq },
			{ ParamID::kHiFreq, &hiFreq },
			{ ParamID::kQ, &q },
			{ ParamID::kMix, &mix },
		};

		WobbleDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.waveLength = GetValue(waveLength, status, isOn),
				.loFreq = GetValue(loFreq, status, isOn),
				.hiFreq = GetValue(hiFreq, status, isOn),
				.q = GetValue(q, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
