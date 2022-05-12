#pragma once
#include <memory>
#include "audio_effect.hpp"

namespace ksmaudio
{
	struct FlangerParams
	{
		float periodSec = 4.0f;
		float delay = 30.0f;
		float depth = 45.0f;
		float feedback = 0.6f;
		float stereoWidth = 0.0f;
		float vol = 0.75f;
		float mix = 0.8f;
	};

	class FlangerDSP;

	class Flanger : public IAudioEffect
	{
	private:
		bool m_bypass = false;
		FlangerParams m_params;
		std::unique_ptr<FlangerDSP> m_dsp;

	public:
		Flanger(std::size_t sampleRate, std::size_t numChannels);

		virtual ~Flanger();

		void process(float* pData, std::size_t dataSize);
	};

}
