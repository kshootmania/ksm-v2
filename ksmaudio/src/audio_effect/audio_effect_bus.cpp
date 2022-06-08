#include "ksmaudio/audio_effect/audio_effect_bus.hpp"

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

	void AudioEffectBus::update(const AudioEffect::Status& status, const std::unordered_map<std::string, ParamValueSetDict>& activeAudioEffects)
	{
		// Update override params
		{
			// "Active -> Inactive"
			for (const std::size_t& idx : m_activeAudioEffectIdxs)
			{
				assert(m_names.size() > idx);
				if (!activeAudioEffects.contains(m_names[idx]))
				{
					assert(m_paramControllers.size() > idx);
					m_paramControllers[idx].clearOverrideParams();
				}
			}

			// "Inactive -> Active" or "Active -> Active"
			m_activeAudioEffectIdxs.clear();
			for (const auto& [audioEffectName, params] : activeAudioEffects)
			{
				if (!m_nameIdxDict.contains(audioEffectName))
				{
					continue;
				}

				const std::size_t idx = m_nameIdxDict.at(audioEffectName);

				assert(m_paramControllers.size() > idx);
				m_paramControllers[idx].setOverrideParams(params);

				m_activeAudioEffectIdxs.insert(idx);
			}
		}

		// Update all audio effects
		for (std::size_t i = 0U; i < m_audioEffects.size(); ++i)
		{
			if (m_paramControllers[i].update(status.sec))
			{
				// When the parameter value set is updated, pass it to the audio effect
				for (const auto& [paramID, valueSet] : m_paramControllers[i].currentParams())
				{
					m_audioEffects[i]->setParamValueSet(paramID, valueSet);
				}
			}

			const bool isOn = activeAudioEffects.contains(m_names[i]);
			m_audioEffects[i]->updateStatus(status, isOn);
		}
	}
}
