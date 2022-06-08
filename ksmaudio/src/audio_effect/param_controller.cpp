#include "ksmaudio/audio_effect/param_controller.hpp"

namespace ksmaudio::AudioEffect
{
	void ParamController::refreshCurrentParams(float timeSec)
	{
		m_currentParams = m_baseParams;

		for (const auto& [paramID, timeline] : m_baseParamChanges)
		{
			if (timeline.hasValue())
			{
				m_currentParams[paramID] = timeline.value();
			}
		}

		for (const auto& [paramID, valueSet] : m_overrideParams)
		{
			m_currentParams[paramID] = valueSet;
		}

		m_isDirty = true;
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

	bool ParamController::update(float timeSec)
	{
		if (timeSec < m_timeSec)
		{
			return std::exchange(m_isDirty, false);
		}

		// Update timelines
		bool baseParamChanged = false;
		for (auto& [_, timeline] : m_baseParamChanges)
		{
			if (timeline.update(timeSec))
			{
				baseParamChanged = true;
			}
		}

		if (baseParamChanged)
		{
			refreshCurrentParams(timeSec);
		}

		m_timeSec = timeSec;

		return std::exchange(m_isDirty, false);
	}

	void ParamController::setOverrideParams(const ParamValueSetDict& overrideParams)
	{
		m_overrideParams = overrideParams;
		refreshCurrentParams(m_timeSec);
	}

	void ParamController::clearOverrideParams()
	{
		m_overrideParams.clear();
		refreshCurrentParams(m_timeSec);
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
}
