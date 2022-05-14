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
		Param reduction = DefineParam("reduction", Type::kSample, "0samples");
		Param mix = DefineParam("mix", Type::kRate, "0%>100%");

		BitcrusherDSPParams render(const Status& status)
		{
			return {
				.reduction = GetValue(reduction, status),
				.mix = GetValue(mix, status),
			};
		}
	};
}
