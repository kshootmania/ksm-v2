#pragma once
#include <array>
#include <memory>
#include "bass.h"
#include "audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	class IAudioEffect
	{
	public:
		virtual ~IAudioEffect() = default;

		virtual void process(float* pData, std::size_t dataSize) = 0;

		virtual void updateStatus(const Status& status) = 0;

		virtual bool setParamValueSet(const std::string& name, const std::string& valueSetStr) = 0;
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
	private:
		bool m_bypass = false;
		Params m_params;
		DSPParams m_dspParams;
		DSP m_dsp;

	public:
		BasicAudioEffect(std::size_t sampleRate, std::size_t numChannels)
			: m_dsp(DSPCommonInfo{ sampleRate, numChannels })
		{
			updateStatus(Status{});
		}

		virtual ~BasicAudioEffect() = default;

		virtual void process(float* pData, std::size_t dataSize) override
		{
			m_dsp.process(pData, dataSize, m_bypass, m_dspParams);
		}

		virtual void updateStatus(const Status& status) override
		{
			m_dspParams = m_params.render(status);
		}

		virtual bool setParamValueSet(const std::string& name, const std::string& valueSetStr) override
		{
			if (m_params.dict.contains(name))
			{
				Param& paramRef = *m_params.dict.at(name);
				bool success;
				paramRef.valueSet = StrToValueSet(paramRef.type, valueSetStr, &success);
				return success;
			}
			else
			{
				return false;
			}
		}
	};
}
