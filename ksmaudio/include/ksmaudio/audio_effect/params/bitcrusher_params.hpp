#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct BitcrusherDSPParams
	{
		float reduction = 10.0f;
		float mix = 1.0f;
	};

	struct BitcrusherParams
	{
		Param reduction = DefineParam(Type::kSample, "0samples");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<std::string, Param*> dict = {
			{ "reduction", &reduction },
			{ "mix", &mix },
		};

		BitcrusherDSPParams render(const Status& status, bool isOn)
		{
			return {
				.reduction = GetValue(reduction, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
