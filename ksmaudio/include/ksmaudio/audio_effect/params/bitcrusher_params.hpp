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
		Param reduction = DefineParam(Type::kSample, "0samples-30samples");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kReduction, &reduction },
			{ ParamID::kMix, &mix },
		};

		BitcrusherDSPParams render(const Status& status, bool isOn)
		{
			return {
				.reduction = GetValue(reduction, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}

		BitcrusherDSPParams renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return render(status, isOn);
		}

		BitcrusherDSPParams renderByLaser(const Status& status, bool isOn)
		{
			return render(status, isOn);
		}
	};
}
