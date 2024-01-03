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
	struct ActiveAudioEffectInvocation
	{
		// 上書きパラメータへのポインタ
		const ParamValueSetDict* pOverrideParams;

		// 音声エフェクトを呼び出したノーツのレーンインデックス
		std::size_t laneIdx;
	};
	using ActiveAudioEffectDict = std::unordered_map<std::size_t, ActiveAudioEffectInvocation>;

    class AudioEffectBus
    {
	private:
		const bool m_isLaser; // TODO: なるべく型で区別したい
		Stream* const m_pStream;
		std::vector<std::unique_ptr<AudioEffect::IAudioEffect>> m_audioEffects;
		std::vector<HDSP> m_hDSPs;
		std::vector<ParamController> m_paramControllers;
		std::unordered_map<std::string, std::size_t> m_nameIdxDict;
		std::unordered_set<std::size_t> m_activeAudioEffectIdxs;

	public:
		AudioEffectBus(bool isLaser, Stream* pStream);

		~AudioEffectBus();

		// ロングFXによる更新
		void updateByFX(const AudioEffect::Status& status, const ActiveAudioEffectDict& activeAudioEffectDict);

		// LASERによる更新
		void updateByLaser(const AudioEffect::Status& status, std::optional<std::size_t> activeAudioEffectIdx);

		template <typename T>
		void emplaceAudioEffect(const std::string& name,
			const std::unordered_map<ParamID, ValueSet>& params = {},
			const std::unordered_map<ParamID, std::map<float, ValueSet>>& paramChanges = {},
			const std::set<float>& updateTriggerTiming = {})
			requires std::derived_from<T, AudioEffect::IAudioEffect>
		{
			if (m_nameIdxDict.contains(name))
			{
				assert(false && "There is already an audio effect of the same name");
				return;
			}

			if constexpr (T::kIsWithTrigger)
			{
				m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels(), m_isLaser, updateTriggerTiming));
			}
			else
			{
				m_audioEffects.push_back(std::make_unique<T>(m_pStream->sampleRate(), m_pStream->numChannels(), m_isLaser));
			}
			const auto& audioEffect = m_audioEffects.back();

			for (const auto& [paramID, valueSet] : params)
			{
				audioEffect->setParamValueSet(paramID, valueSet);
			}

			m_nameIdxDict.emplace(name, m_audioEffects.size() - 1U);

			const HDSP hDSP = m_pStream->addAudioEffect(audioEffect.get(), T::kPriority);
			m_hDSPs.push_back(hDSP);

			// ここで、paramsを渡すのではなくparamValueSetDict()で改めて取得しているのは、Dict内に暗黙に定義されるデフォルト値も入れる必要があるため
			m_paramControllers.emplace_back(audioEffect->paramValueSetDict(), paramChanges);
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

		void setBypass(bool bypass);

		bool audioEffectContainsName(const std::string& name) const;

		std::size_t audioEffectNameToIdx(const std::string& name) const;
    };
}
