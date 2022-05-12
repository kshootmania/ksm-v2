#pragma once
#include "audio_effect.hpp"

namespace ksmaudio
{
	struct BitcrusherParams
	{
		float reduction = 10.0f;
		float mix = 1.0f;
	};

	class BitcrusherDSP;

	class Bitcrusher : public IAudioEffect
	{
	private:
		bool m_bypass = false;
		BitcrusherParams m_params;
		std::unique_ptr<BitcrusherDSP> m_dsp;

	public:
		Bitcrusher(std::size_t sampleRate, std::size_t numChannels);

		void process(float* pData, std::size_t dataSize);
	};
}
