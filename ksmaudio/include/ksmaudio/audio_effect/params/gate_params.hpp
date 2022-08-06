#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct GateDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: Negative value will be just ignored
		float waveLength = 0.0f;
		float rate = 0.5f;
		float mix = 0.9f;
	};

	struct GateParams
	{
		Param waveLength = DefineParam(Type::kLength, "0");
		Param rate = DefineParam(Type::kRate, "50%");
		Param mix = DefineParam(Type::kRate, "0%>90%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kWaveLength, &waveLength },
			{ ParamID::kRate, &rate },
			{ ParamID::kMix, &mix },
		};

	private:
		// TODO: Make a class for this
		std::set<float> m_updateTriggerTiming;
		std::set<float>::const_iterator m_updateTriggerTimingCursor = m_updateTriggerTiming.begin();

		float getSecUntilTrigger(float currentSec)
		{
			if (m_updateTriggerTimingCursor == m_updateTriggerTiming.end())
			{
				return -1.0f; // Negative value will be ignored in DSP
			}

			// Advance m_updateTriggerTimingCursor to currentSec
			if (*m_updateTriggerTimingCursor < currentSec)
			{
				do
				{
					++m_updateTriggerTimingCursor;
				} while (m_updateTriggerTimingCursor != m_updateTriggerTiming.end() && *m_updateTriggerTimingCursor < currentSec);

				if (m_updateTriggerTimingCursor == m_updateTriggerTiming.end())
				{
					return -1.0f; // Negative value will be ignored in DSP
				}
				else
				{
					return 0.0f; // Update immediately (TODO: avoid extra update)
				}
			}

			return *m_updateTriggerTimingCursor - currentSec;
		}

	public:
		// Note: For gate audio effects, the update trigger timing is the bar line timing
		void setUpdateTriggerTiming(const std::set<float>& timing)
		{
			m_updateTriggerTiming = timing;
			m_updateTriggerTimingCursor = m_updateTriggerTiming.begin();
		}

		GateDSPParams render(const Status& status, bool isOn)
		{
			return {
				.secUntilTrigger = getSecUntilTrigger(status.sec),
				.waveLength = GetValue(waveLength, status, isOn),
				.rate = GetValue(rate, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
