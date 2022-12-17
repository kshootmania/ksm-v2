#pragma once
#include <array>
#include <memory>
#include <cassert>
#include "bass.h"
#include "audio_effect_param.hpp"
#include "detail/update_trigger_timeline.hpp"

namespace ksmaudio::AudioEffect
{
	class IAudioEffect
	{
	public:
		virtual ~IAudioEffect() = default;

		virtual void process(float* pData, std::size_t dataSize) = 0;

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) = 0;

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSetStr) = 0;

		virtual std::unordered_map<ParamID, ValueSet> paramValueSetDict() const = 0;

		virtual void setBypass(bool bypass) = 0;
	};

	struct DSPCommonInfo
	{
		bool isUnsupported;

		std::size_t sampleRate;

		float sampleRateScale;

		std::size_t numChannels;

		constexpr DSPCommonInfo(std::size_t sampleRate, std::size_t numChannels)
			: isUnsupported(numChannels == 0U || numChannels >= 3U) // Supports stereo and mono only
			, sampleRate(sampleRate)
			, sampleRateScale(sampleRate / 44100.0f)
			, numChannels(numChannels)
		{
		}
	};

	template <typename Params, typename DSP, typename DSPParams>
	class BasicAudioEffect : public IAudioEffect
	{
	protected:
		bool m_bypass = false;
		Params m_params;
		DSPParams m_dspParams;
		DSP m_dsp;

		// isParamUpdated決定用の更新回数カウンタ
		int m_statusUpdateCount = 0;
		int m_prevStatusUpdateCountInProcessThread = 0;

	public:
		static constexpr bool kIsWithTrigger = false;

		BasicAudioEffect(std::size_t sampleRate, std::size_t numChannels)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
		{
		}

		virtual ~BasicAudioEffect() = default;

		// この関数のみ他の関数とは別のスレッドから呼ばれるので注意
		virtual void process(float* pData, std::size_t dataSize) override
		{
			// updateStatusが呼ばれた後の初回のみトリガ更新可能
			const bool isParamUpdated = m_statusUpdateCount != m_prevStatusUpdateCountInProcessThread;
			m_prevStatusUpdateCountInProcessThread = m_statusUpdateCount;

			m_dsp.process(pData, dataSize, m_bypass, m_dspParams, isParamUpdated);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			m_dspParams = m_params.render(status, laneIdx);

			++m_statusUpdateCount;
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
		}

		virtual std::unordered_map<ParamID, ValueSet> paramValueSetDict() const override
		{
			std::unordered_map<ParamID, ValueSet> dict;
			for (const auto& [paramID, pParam] : m_params.dict)
			{
				dict.emplace(paramID, pParam->valueSet);
			}
			return dict;
		}

		virtual void setBypass(bool bypass) override
		{
			m_bypass = bypass;
		}
	};

	template <typename Params, typename DSP, typename DSPParams>
	class BasicAudioEffectWithTrigger : public IAudioEffect
	{
	protected:
		bool m_bypass = false;
		Params m_params;
		DSPParams m_dspParams;
		DSP m_dsp;
		detail::UpdateTriggerTimeline m_updateTriggerTimeline;

		// isParamUpdated決定用の更新回数カウンタ
		int m_statusUpdateCount = 0;
		int m_prevStatusUpdateCountInProcessThread = 0;

	public:
		static constexpr bool kIsWithTrigger = true;

		BasicAudioEffectWithTrigger(std::size_t sampleRate, std::size_t numChannels, const std::set<float>& updateTriggerTiming)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
			, m_updateTriggerTimeline(updateTriggerTiming)
		{
		}

		virtual ~BasicAudioEffectWithTrigger() = default;

		// この関数のみ他の関数とは別のスレッドから呼ばれるので注意
		virtual void process(float* pData, std::size_t dataSize) override
		{
			// updateStatusが呼ばれた後の初回のみトリガ更新可能
			const bool isParamUpdated = m_statusUpdateCount != m_prevStatusUpdateCountInProcessThread;
			m_prevStatusUpdateCountInProcessThread = m_statusUpdateCount;

			m_dsp.process(pData, dataSize, m_bypass, m_dspParams, isParamUpdated);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			m_dspParams = m_params.render(status, laneIdx);

			m_updateTriggerTimeline.update(status.sec);
			m_dspParams.secUntilTrigger = m_updateTriggerTimeline.secUntilTrigger();

			++m_statusUpdateCount;
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
		}

		virtual std::unordered_map<ParamID, ValueSet> paramValueSetDict() const override
		{
			std::unordered_map<ParamID, ValueSet> dict;
			for (const auto& [paramID, pParam] : m_params.dict)
			{
				dict.emplace(paramID, pParam->valueSet);
			}
			return dict;
		}

		virtual void setBypass(bool bypass) override
		{
			m_bypass = bypass;
		}
	};
}
