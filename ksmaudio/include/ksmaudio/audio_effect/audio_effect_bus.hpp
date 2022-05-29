#pragma once
#include <memory>
#include <array>
#include <set>
#include <map>
#include <unordered_map>
#include <concepts>
#include <optional>
#include <stdexcept>
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

	class ParamController // TODO: review overhead
	{
	public:
		using ValueSetDict = std::unordered_map<std::string, ValueSet>;
		using ValueSetTimelineDict = std::unordered_map<std::string, Timeline<ValueSet>>;

		using OverrideParamChanges = std::array<Timeline<std::optional<ValueSetDict>>, kOverrideMax>;

	private:
		ValueSetTimelineDict m_baseParamChanges; // For "param_change"
		OverrideParamChanges m_overrideParamChanges; // For "long_event"

		ValueSetDict m_baseParams;
		ValueSetDict m_currentParams;

		// Note: It is assumed that kOverrideMax is 2. Otherwise, a stack is required here.
		std::size_t m_lastActiveOverrideIdx = 0U;
		std::array<bool, kOverrideMax> m_overrideActive = { false, false };

		float m_timeSec = kPastTimeSec;

		void refreshCurrentParams(float timeSec);

	public:
		ParamController(const ValueSetDict& baseParams,
			const std::unordered_map<std::string, std::map<float, ValueSet>>& baseParamChanges,
			const std::array<std::map<float, std::optional<ValueSetDict>>, kOverrideMax>& overrideParamChanges);

		void update(float timeSec, const std::array<bool, kOverrideMax>& overrideActive);

		const std::unordered_map<std::string, ValueSet> currentParams() const;
	};

    class AudioEffectBus
    {
	private:
		Stream* m_pStream;
		std::vector<std::unique_ptr<AudioEffect::IAudioEffect>> m_audioEffects;
		std::vector<HDSP> m_hDSPs;
		std::vector<ParamController> m_paramControllers;
		std::vector<std::string> m_names;
		std::unordered_map<std::string, std::size_t> m_nameIdxDict;

	public:
		AudioEffectBus() = default;

		explicit AudioEffectBus(Stream* pStream);

		~AudioEffectBus();

		void update(const AudioEffect::Status& status, const std::array<bool, kOverrideMax>& overrideActive, const std::set<std::string>& onAudioEffectNames);

		template <typename T>
		void emplaceAudioEffect(const std::string& name,
			const std::unordered_map<std::string, std::string>& params = {},
			const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges = {},
			const std::array<std::map<float, std::optional<std::unordered_map<std::string, std::string>>>, kOverrideMax>& overrideParamChanges = {},
			const std::set<float>& updateTriggerTiming = {})
			requires std::derived_from<T, AudioEffect::IAudioEffect>
		{
			using ValueSetDict = ParamController::ValueSetDict;

			if (m_nameIdxDict.contains(name))
			{
				// TODO: Report warning
				return;
			}

			m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels()));
			const auto& audioEffect = m_audioEffects.back();

			std::unordered_map<std::string, ValueSet> paramsAsValueSet;
			for (const auto& [paramName, valueSetStr] : params)
			{
				paramsAsValueSet.emplace(paramName, StrToValueSet(audioEffect->paramTypeOf(paramName), valueSetStr));
			}
			for (const auto& [paramName, valueSet] : paramsAsValueSet)
			{
				audioEffect->setParamValueSet(paramName, valueSet);
			}
			audioEffect->updateStatus(AudioEffect::Status{}, false);

			if constexpr (std::is_base_of_v<AudioEffect::IUpdateTrigger, T>)
			{
				dynamic_cast<T*>(audioEffect.get())->setUpdateTriggerTiming(updateTriggerTiming);
			}

			m_names.push_back(name);
			m_nameIdxDict.emplace(name, m_audioEffects.size());

			const HDSP hDSP = m_pStream->addAudioEffect(audioEffect.get(), 0); // TODO: priority
			m_hDSPs.push_back(hDSP);

			std::unordered_map<std::string, std::map<float, ValueSet>> baseParamChangesAsValueSet;
			for (const auto& [paramName, map] : paramChanges)
			{
				std::map<float, ValueSet>& targetMap = baseParamChangesAsValueSet[paramName];
				for (const auto& [sec, valueSetStr] : map)
				{
					targetMap.emplace(sec, StrToValueSet(audioEffect->paramTypeOf(paramName), valueSetStr));
				}
			}

			std::array<std::map<float, std::optional<ValueSetDict>>, kOverrideMax> overrideParamChangesAsValueSet;
			for (std::size_t i = 0U; i < kOverrideMax; ++i)
			{
				std::map<float, std::optional<ValueSetDict>>& targetMap = overrideParamChangesAsValueSet[i];
				for (const auto& [sec, overrideParams] : overrideParamChanges[i])
				{
					if (overrideParams.has_value())
					{
						ValueSetDict& targetParams = targetMap[sec].emplace();
						for (const auto& [paramName, valueSetStr] : *overrideParams)
						{
							targetParams.emplace(paramName, StrToValueSet(audioEffect->paramTypeOf(paramName), valueSetStr));
						}
					}
					else
					{
						targetMap.emplace(sec, std::nullopt);
					}
				}
			}

			m_paramControllers.emplace_back(paramsAsValueSet, baseParamChangesAsValueSet, overrideParamChangesAsValueSet);
		}

		ParamController& paramControllerAt(const std::string& name);
    };
}
