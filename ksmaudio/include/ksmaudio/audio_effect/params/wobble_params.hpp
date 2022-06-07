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

	private:
		// TODO: Make a class for this
		std::set<float> m_updateTriggerTiming;
		std::set<float>::const_iterator m_updateTriggerTimingCursor = m_updateTriggerTiming.begin();

		float getSecUntilTrigger(float currentSec)
		{
			while (true)
			{
				if (m_updateTriggerTimingCursor == m_updateTriggerTiming.end())
				{
					return -1.0f; // Negative value will be ignored in DSP
				}

				if (*m_updateTriggerTimingCursor < currentSec)
				{
					++m_updateTriggerTimingCursor;
					return 0.0f;
				}
				else
				{
					break;
				}
			}
			return *m_updateTriggerTimingCursor - currentSec;
		}

	public:
		// Note: For wobble audio effects, the update trigger timing is the bar line timing
		void setUpdateTriggerTiming(const std::set<float>& timing)
		{
			m_updateTriggerTiming = timing;
			m_updateTriggerTimingCursor = m_updateTriggerTiming.begin();
		}

		WobbleDSPParams render(const Status& status, bool isOn)
		{
			return {
				.secUntilTrigger = getSecUntilTrigger(status.sec),
				.waveLength = GetValue(waveLength, status, isOn),
				.loFreq = GetValue(loFreq, status, isOn),
				.hiFreq = GetValue(hiFreq, status, isOn),
				.q = GetValue(q, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
