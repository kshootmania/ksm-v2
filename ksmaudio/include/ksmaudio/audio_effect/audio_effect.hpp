#pragma once
#include <array>
#include <memory>
#include <mutex>
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
		std::mutex m_mutex;

	public:
		static constexpr bool kIsWithTrigger = false;

		BasicAudioEffect(std::size_t sampleRate, std::size_t numChannels)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
		{
		}

		virtual ~BasicAudioEffect() = default;

		// この関数のみ他の関数とは別のスレッドからも呼ばれるので注意
		virtual void process(float* pData, std::size_t dataSize) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_dsp.process(pData, dataSize, m_bypass, m_dspParams);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_dspParams = m_params.render(status, laneIdx);
			m_dsp.updateParams(m_dspParams);
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			// updateStatusと同じスレッドで実行され、processとの同一変数操作もないのでlock_guard不要

			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
		}

		virtual std::unordered_map<ParamID, ValueSet> paramValueSetDict() const override
		{
			// updateStatusと同じスレッドで実行され、processとの同一変数操作もないのでlock_guard不要

			std::unordered_map<ParamID, ValueSet> dict;
			for (const auto& [paramID, pParam] : m_params.dict)
			{
				dict.emplace(paramID, pParam->valueSet);
			}
			return dict;
		}

		virtual void setBypass(bool bypass) override
		{
			if (m_bypass != bypass)
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				m_bypass = bypass;
			}
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
		std::mutex m_mutex;

	public:
		static constexpr bool kIsWithTrigger = true;

		BasicAudioEffectWithTrigger(std::size_t sampleRate, std::size_t numChannels, const std::set<float>& updateTriggerTiming)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
			, m_updateTriggerTimeline(updateTriggerTiming)
		{
		}

		virtual ~BasicAudioEffectWithTrigger() = default;

		// この関数のみ他の関数とは別のスレッドからも呼ばれるので注意
		virtual void process(float* pData, std::size_t dataSize) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_dsp.process(pData, dataSize, m_bypass, m_dspParams);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_dspParams = m_params.render(status, laneIdx);

			m_updateTriggerTimeline.update(status.sec);
			m_dspParams.secUntilTrigger = m_updateTriggerTimeline.secUntilTrigger();

			m_dsp.updateParams(m_dspParams);
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			// updateStatusと同じスレッドで実行され、processとの同一変数操作もないのでlock_guard不要

			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
		}

		virtual std::unordered_map<ParamID, ValueSet> paramValueSetDict() const override
		{
			// updateStatusと同じスレッドで実行され、processとの同一変数操作もないのでlock_guard不要

			std::unordered_map<ParamID, ValueSet> dict;
			for (const auto& [paramID, pParam] : m_params.dict)
			{
				dict.emplace(paramID, pParam->valueSet);
			}
			return dict;
		}

		virtual void setBypass(bool bypass) override
		{
			if (m_bypass != bypass)
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				m_bypass = bypass;
			}
		}
	};
}
