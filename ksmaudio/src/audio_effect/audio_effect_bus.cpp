#include "ksmaudio/audio_effect/audio_effect_bus.hpp"

namespace ksmaudio::AudioEffect
{
	void ParamController::refreshCurrentParams(float timeSec)
	{
		m_currentParams = m_baseParams;
	
		// Note: It is assumed that m_baseParams contains all the keys contained in the following param changes.
		//       Otherwise, the parameter values will not revert to default after the override ends.

		for (const auto& [paramName, timeline] : m_baseParamChanges)
		{
			if (timeline.hasValue())
			{
				m_currentParams[paramName] = timeline.value();
			}
		}

		if (m_overrideActive[m_lastActiveOverrideIdx])
		{
			const auto& timeline = m_overrideParamChanges[m_lastActiveOverrideIdx];
			if (timeline.hasValue())
			{
				for (const auto& [paramName, valueSet] : *timeline.value())
				{
					m_currentParams[paramName] = valueSet;
				}
			}
		}
	}

	ParamController::ParamController(const ValueSetDict& baseParams,
		const std::unordered_map<std::string, std::map<float, ValueSet>>& baseParamChanges,
		const std::array<std::map<float, std::optional<ValueSetDict>>, kOverrideMax>& overrideParamChanges)
		: m_baseParams(baseParams)
		, m_overrideParamChanges{
			Timeline<std::optional<ValueSetDict>>{ overrideParamChanges[0] },
			Timeline<std::optional<ValueSetDict>>{ overrideParamChanges[1] } }
	{
		for (const auto& [paramName, map] : baseParamChanges)
		{
			m_baseParamChanges.emplace(std::piecewise_construct, std::make_tuple(paramName), std::make_tuple(map));
		}
	}

	void ParamController::update(float timeSec, const std::array<bool, kOverrideMax>& overrideActive)
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
		for (auto& timeline : m_overrideParamChanges)
		{
			if (timeline.update(timeSec))
			{
				isDirty = true;
			}
		}

		// Update override status
		for (std::size_t i = 0U; i < kOverrideMax; ++i)
		{
			if (overrideActive[i] && !m_overrideActive[i])
			{
				m_lastActiveOverrideIdx = i;
				isDirty = true;
			}
			else if (!overrideActive[i] && m_overrideActive[i] && m_lastActiveOverrideIdx == i && overrideActive[1U - i])
			{
				m_lastActiveOverrideIdx = 1U - i;
				isDirty = true;
			}
		}
		m_overrideActive = overrideActive;

		if (isDirty)
		{
			refreshCurrentParams(timeSec);
		}

		m_timeSec = timeSec;
	}

	const std::unordered_map<std::string, ValueSet> ParamController::currentParams() const
	{
		return m_currentParams;
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

	void AudioEffectBus::update(const AudioEffect::Status& status, const std::array<bool, kOverrideMax>& overrideActive, const std::set<std::string>& onAudioEffectNames)
	{
		for (std::size_t i = 0U; i < m_audioEffects.size(); ++i)
		{
			m_paramControllers[i].update(status.sec, overrideActive);
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
