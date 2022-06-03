#include "ksmaudio/audio_effect/audio_effect_bus.hpp"

namespace ksmaudio::AudioEffect
{
	void ParamController::refreshCurrentParams(float timeSec)
	{
		m_currentParams = m_baseParams;
	
		// Note: It is assumed that m_baseParams contains all the keys contained in m_baseParamChanges.
		//       Otherwise, the parameter values will not revert to default after the override ends.

		for (const auto& [paramID, timeline] : m_baseParamChanges)
		{
			if (timeline.hasValue())
			{
				m_currentParams[paramID] = timeline.value();
			}
		}
	}

	ParamController::ParamController(const ParamValueSetDict& baseParams,
		const std::unordered_map<ParamID, std::map<float, ValueSet>>& baseParamChanges)
		: m_baseParams(baseParams)
	{
		for (const auto& [paramID, map] : baseParamChanges)
		{
			m_baseParamChanges.emplace(std::piecewise_construct, std::make_tuple(paramID), std::make_tuple(map));
		}
	}

	void ParamController::update(float timeSec)
	{
		if (timeSec < m_timeSec)
		{
			return;
		}

		// Update timelines
		bool isDirty = false;
		for (auto& [_, timeline] : m_baseParamChanges)
		{
			if (timeline.update(timeSec))
			{
				isDirty = true;
			}
		}

		if (isDirty)
		{
			refreshCurrentParams(timeSec);
		}

		m_timeSec = timeSec;
	}

	const ParamValueSetDict& ParamController::currentParams() const
	{
		return m_currentParams;
	}

	ParamValueSetDict StrDictToParamValueSetDict(const std::unordered_map<std::string, std::string>& strDict)
	{
		std::unordered_map<ParamID, ValueSet> paramValueSetDict;
		for (const auto& [paramName, valueSetStr] : strDict)
		{
			if (kStrToParamID.contains(paramName))
			{
				const ParamID paramID = kStrToParamID.at(paramName);
				const Type type = kParamIDType.at(paramID);
				paramValueSetDict.emplace(paramID, StrToValueSet(type, valueSetStr));
			}
		}
		return paramValueSetDict;
	}

	std::unordered_map<ParamID, std::map<float, ValueSet>> StrTimelineToValueSetTimeline(const std::unordered_map<std::string, std::map<float, std::string>>& strTimeline)
	{
		std::unordered_map<ParamID, std::map<float, ValueSet>> valueSetTimeline;
		for (const auto& [paramName, map] : strTimeline)
		{
			if (kStrToParamID.contains(paramName))
			{
				const ParamID paramID = kStrToParamID.at(paramName);
				const Type type = kParamIDType.at(paramID);
				auto& targetMap = valueSetTimeline[paramID];
				for (const auto& [sec, valueSetStr] : map)
				{
					targetMap.emplace(sec, StrToValueSet(type, valueSetStr));
				}
			}
		}
		return valueSetTimeline;
	}

	AudioEffectBus::AudioEffectBus(Stream* pStream)
		: m_pStream(pStream)
	{
	}

	AudioEffectBus::~AudioEffectBus()
    {
		for (const auto& hDSP : m_hDSPs)
		{
			m_pStream->removeAudioEffect(hDSP);
		}
    }

	void AudioEffectBus::update(const AudioEffect::Status& status, const std::set<std::string>& onAudioEffectNames)
	{
		for (std::size_t i = 0U; i < m_audioEffects.size(); ++i)
		{
			m_paramControllers[i].update(status.sec);
			const bool isOn = onAudioEffectNames.contains(m_names[i]); // costly?
			m_audioEffects[i]->updateStatus(status, isOn);
		}
	}

	ParamController& AudioEffectBus::paramControllerAt(const std::string& name)
	{
		assert(m_nameIdxDict.contains(name));
		const std::size_t idx = m_nameIdxDict.at(name);
		assert(idx < m_paramControllers.size());
		return m_paramControllers[idx];
	}
}
