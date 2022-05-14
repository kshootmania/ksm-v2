#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/flanger_params.hpp"

namespace ksmaudio::AudioEffect
{
	class FlangerDSP;

	class Flanger : public IAudioEffect
	{
	private:
		bool m_bypass = false;
		FlangerParams m_params;
		FlangerDSPParams m_dspParams;
		std::unique_ptr<FlangerDSP> m_dsp;

	public:
		Flanger(std::size_t sampleRate, std::size_t numChannels);

		virtual ~Flanger();

		void process(float* pData, std::size_t dataSize);
	};
}
