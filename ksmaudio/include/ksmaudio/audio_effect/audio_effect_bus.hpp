#pragma once
#include <memory>
#include <array>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <concepts>
#include "bass.h"
#include "audio_effect.hpp"
#include "param_controller.hpp"
#include "ksmaudio/stream.hpp"

namespace ksmaudio::AudioEffect
{

    class AudioEffectBus
    {
	private:
		Stream* m_pStream;
		std::vector<std::unique_ptr<AudioEffect::IAudioEffect>> m_audioEffects;
		std::vector<HDSP> m_hDSPs;
		std::vector<ParamController> m_paramControllers;
		std::vector<std::string> m_names;
		std::unordered_map<std::string, std::size_t> m_nameIdxDict;
		std::unordered_set<std::size_t> m_activeAudioEffectIdxs;

	public:
		AudioEffectBus() = default;

		explicit AudioEffectBus(Stream* pStream);

		~AudioEffectBus();

		void update(const AudioEffect::Status& status, const std::unordered_map<std::string, ParamValueSetDict>& activeAudioEffects);

		template <typename T>
		void emplaceAudioEffect(const std::string& name,
			const std::unordered_map<ParamID, ValueSet>& params = {},
			const std::unordered_map<ParamID, std::map<float, ValueSet>>& paramChanges = {},
			const std::set<float>& updateTriggerTiming = {})
			requires std::derived_from<T, AudioEffect::IAudioEffect>
		{
			if (m_nameIdxDict.contains(name))
			{
				// There is already an audio effect of the same name
				// TODO: Report warning
				return;
			}

			if constexpr (T::kIsWithTrigger)
			{
				m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels(), updateTriggerTiming));
			}
			else
			{
				m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels()));
			}
			const auto& audioEffect = m_audioEffects.back();

			for (const auto& [paramID, valueSet] : params)
			{
				audioEffect->setParamValueSet(paramID, valueSet);
			}
			audioEffect->updateStatus(AudioEffect::Status{}, false);

			m_names.push_back(name);
			m_nameIdxDict.emplace(name, m_audioEffects.size() - 1U);

			const HDSP hDSP = m_pStream->addAudioEffect(audioEffect.get(), 0); // TODO: priority
			m_hDSPs.push_back(hDSP);

			m_paramControllers.emplace_back(params, paramChanges);
		}

		template <typename T>
		void emplaceAudioEffect(const std::string& name,
			const std::unordered_map<std::string, std::string>& params,
			const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges = {},
			const std::set<float>& updateTriggerTiming = {})
			requires std::derived_from<T, AudioEffect::IAudioEffect>
		{
			emplaceAudioEffect<T>(name, StrDictToParamValueSetDict(params), StrTimelineToValueSetTimeline(paramChanges), updateTriggerTiming);
		}

		void setBypass(bool bypass)
		{
			for (const auto& audioEffect : m_audioEffects)
			{
				audioEffect->setBypass(bypass);
			}
		}
    };
}
