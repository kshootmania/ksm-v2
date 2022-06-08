#pragma once
#include <memory>
#include <array>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <concepts>
#include <optional>
#include <cassert>
#include "bass.h"
#include "audio_effect.hpp"
#include "ksmaudio/stream.hpp"

namespace ksmaudio::AudioEffect
{
	constexpr std::size_t kOverrideMax = 2U;

	constexpr float kPastTimeSec = -1000.0f;

	namespace detail
	{
		template <typename T, typename U>
		auto CurrentAt(const std::map<T, U>& map, T key)
		{
			auto itr = map.upper_bound(key);
			if (itr != map.begin())
			{
				--itr;
			}
			return itr;
		}

		template <typename T>
		class Timeline
		{
		private:
			const std::map<float, T> m_map;
			std::map<float, T>::const_iterator m_cursorItr;
			std::map<float, T>::const_iterator m_nextCursorItr;
			float m_timeSec = kPastTimeSec;

		public:
			explicit Timeline(const std::map<float, T>& map)
				: m_map(map)
				, m_cursorItr(m_map.cbegin())
				, m_nextCursorItr(m_map.empty() ? m_map.cend() : std::next(m_map.cbegin()))
			{
			}

			bool update(float timeSec)
			{
				if (timeSec < m_timeSec) [[unlikely]]
				{
					// Avoid backtracking
					return false;
				}

				m_timeSec = timeSec;

				if (m_nextCursorItr == m_map.cend())
				{
					return false;
				}

				if (m_nextCursorItr->first > m_timeSec)
				{
					return false;
				}

				const auto itr = detail::CurrentAt(m_map, m_timeSec);
				m_cursorItr = itr;
				m_nextCursorItr = itr == m_map.cend() ? itr : std::next(itr);

				return false;
			}

			bool hasValue() const
			{
				return m_cursorItr != m_map.cend() && m_cursorItr->first <= m_timeSec;
			}

			const T& value() const
			{
				assert(hasValue());
				return m_cursorItr->second;
			}
		};
	}

	class ParamController
	{
	private:
		const ParamValueSetDict m_baseParams; // For "def" in kson
		std::unordered_map<ParamID, detail::Timeline<ValueSet>> m_baseParamChanges; // For "param_change" in kson
		ParamValueSetDict m_overrideParams; // For "long_event" in kson

		ParamValueSetDict m_currentParams;

		float m_timeSec = kPastTimeSec;

		bool m_isDirty = false;

		void refreshCurrentParams(float timeSec);

	public:
		ParamController(const ParamValueSetDict& baseParams,
			const std::unordered_map<ParamID, std::map<float, ValueSet>>& baseParamChanges);

		bool update(float timeSec);

		void setOverrideParams(const ParamValueSetDict& overrideParams);

		void clearOverrideParams();

		const ParamValueSetDict& currentParams() const;
	};

	ParamValueSetDict StrDictToParamValueSetDict(const std::unordered_map<std::string, std::string>& strDict);

	std::unordered_map<ParamID, std::map<float, ValueSet>> StrTimelineToValueSetTimeline(const std::unordered_map<std::string, std::map<float, std::string>>& strTimeline);

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

			m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels()));
			const auto& audioEffect = m_audioEffects.back();

			for (const auto& [paramID, valueSet] : params)
			{
				audioEffect->setParamValueSet(paramID, valueSet);
			}
			audioEffect->updateStatus(AudioEffect::Status{}, false);

			if constexpr (std::is_base_of_v<AudioEffect::IUpdateTrigger, T>)
			{
				dynamic_cast<T*>(audioEffect.get())->setUpdateTriggerTiming(updateTriggerTiming);
			}

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
