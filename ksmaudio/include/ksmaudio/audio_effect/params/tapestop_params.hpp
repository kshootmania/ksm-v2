#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct TapestopDSPParams
	{
		float speed = 0.5f;
		bool trigger = false;
		bool reset = false; // triggerがoffからonになった瞬間のみtrueになる
		float mix = 1.0f;
	};

	struct TapestopParams
	{
		Param speed = DefineParam(Type::kRate, "50%");
		TapestopTriggerParam trigger = DefineTapestopTriggerParam("off>on");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kSpeed, &speed },
			{ ParamID::kTrigger, trigger.innerParamPtr() },
			{ ParamID::kMix, &mix },
		};

		TapestopDSPParams renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.speed = GetValue(speed, status, isOn),
				.trigger = trigger.renderByFX(status, laneIdx),
				.reset = trigger.getResetValue(),
				.mix = GetValue(mix, status, isOn),
			};
		}

		TapestopDSPParams renderByLaser(const Status& status, bool isOn)
		{
			return {
				.speed = GetValue(speed, status, isOn),
				.trigger = trigger.renderByLaser(status, isOn),
				.reset = trigger.getResetValue(),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
