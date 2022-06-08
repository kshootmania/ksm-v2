#include "ksmaudio/audio_effect/audio_effect_bus.hpp"

namespace ksmaudio::AudioEffect
{
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
