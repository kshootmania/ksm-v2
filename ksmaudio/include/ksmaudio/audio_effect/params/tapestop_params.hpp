#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct TapestopDSPParams
	{
		float speed = 0.5f;
		bool trigger = false;
		float mix = 1.0f;
	};

	struct TapestopParams
	{
		Param speed = DefineParam(Type::kRate, "50%");
		Param trigger = DefineParam(Type::kSwitch, "off>on");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kSpeed, &speed },
			{ ParamID::kTrigger, &trigger },
			{ ParamID::kMix, &mix },
		};

		TapestopDSPParams render(const Status& status, bool isOn)
		{
			return {
				.speed = GetValue(speed, status, isOn),
				.trigger = GetValueAsBool(trigger, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
