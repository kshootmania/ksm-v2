#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct RetriggerDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: Negative value will be just ignored
		float waveLength = 0.0f;
		float rate = 0.7f;
		float mix = 1.0f;
	};

	struct RetriggerParams
	{
		Param updatePeriod = DefineParam(Type::kLength, "1/2");
		Param waveLength = DefineParam(Type::kLength, "0");
		Param rate = DefineParam(Type::kRate, "70%");
		Param updateTrigger = DefineParam(Type::kSwitch, "off");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kUpdatePeriod, &updatePeriod },
			{ ParamID::kWaveLength, &waveLength },
			{ ParamID::kRate, &rate },
			{ ParamID::kUpdateTrigger, &updateTrigger },
			{ ParamID::kMix, &mix },
		};

		RetriggerDSPParams render(const Status& status, bool isOn)
		{
			// TODO: Use updateTrigger
			/*const bool updateTriggerNow = GetValue(updateTrigger, status, isOn) == 1.0f;
			float secUntilTrigger = getSecUntilTrigger(status.sec);
			if (!m_updateTriggerPrev && updateTriggerNow)
			{
				secUntilTrigger = 0.0f; // FIXME: Set back to false
			}
			m_updateTriggerPrev = updateTriggerNow;*/

			return {
				//.secUntilTrigger = secUntilTrigger,
				.waveLength = GetValue(waveLength, status, isOn),
				.rate = GetValue(rate, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
