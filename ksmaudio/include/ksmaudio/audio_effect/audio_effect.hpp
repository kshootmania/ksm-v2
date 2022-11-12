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

	public:
		static constexpr bool kIsWithTrigger = false;

		BasicAudioEffect(std::size_t sampleRate, std::size_t numChannels)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
		{
		}

		virtual ~BasicAudioEffect() = default;

		virtual void process(float* pData, std::size_t dataSize) override
		{
			m_dsp.process(pData, dataSize, m_bypass, m_dspParams);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			m_dspParams = m_params.render(status, laneIdx);
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
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

	public:
		static constexpr bool kIsWithTrigger = true;

		BasicAudioEffectWithTrigger(std::size_t sampleRate, std::size_t numChannels, const std::set<float>& updateTriggerTiming)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
			, m_dspParams(m_params.render(Status{}, std::nullopt))
			, m_updateTriggerTimeline(updateTriggerTiming)
		{
		}

		virtual ~BasicAudioEffectWithTrigger() = default;

		virtual void process(float* pData, std::size_t dataSize) override
		{
			m_dsp.process(pData, dataSize, m_bypass, m_dspParams);
		}

		virtual void updateStatus(const Status& status, std::optional<std::size_t> laneIdx) override
		{
			m_dspParams = m_params.render(status, laneIdx);

			m_updateTriggerTimeline.update(status.sec);
			m_dspParams.secUntilTrigger = m_updateTriggerTimeline.secUntilTrigger();
		}

		virtual void setParamValueSet(ParamID paramID, const ValueSet& valueSet) override
		{
			if (m_params.dict.contains(paramID))
			{
				m_params.dict.at(paramID)->valueSet = valueSet;
			}
		}

		virtual void setBypass(bool bypass) override
		{
			m_bypass = bypass;
		}
	};
}
