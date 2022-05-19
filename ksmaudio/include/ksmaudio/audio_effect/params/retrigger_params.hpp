#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct RetriggerDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: Negative value is just ignored
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

		const std::unordered_map<std::string, Param*> dict = {
			{ "update_period", &updatePeriod },
			{ "wave_length", &waveLength },
			{ "rate", &rate },
			{ "update_trigger", &updateTrigger },
			{ "mix", &mix },
		};

	private:
		float m_secUntilTrigger = -1.0f;
		bool m_updateTriggerPrev = false;

	public:
		void setSecUntilTrigger(float sec)
		{
			m_secUntilTrigger = sec;
		}

		RetriggerDSPParams render(const Status& status)
		{
			const bool updateTriggerNow = GetValue(updateTrigger, status) == 1.0f;
			if (!m_updateTriggerPrev && updateTriggerNow)
			{
				m_secUntilTrigger = 0.0f;
			}
			m_updateTriggerPrev = updateTriggerNow;

			return {
				.secUntilTrigger = std::exchange(m_secUntilTrigger, -1.0f),
				.waveLength = GetValue(waveLength, status),
				.rate = GetValue(rate, status),
				.mix = GetValue(mix, status),
			};
		}
	};
}
