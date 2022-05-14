#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/bitcrusher_params.hpp"

namespace ksmaudio::AudioEffect
{
	class BitcrusherDSP;

	class Bitcrusher : public IAudioEffect
	{
	private:
		bool m_bypass = false;
		BitcrusherParams m_params;
		BitcrusherDSPParams m_dspParams;
		std::unique_ptr<BitcrusherDSP> m_dsp;

	public:
		Bitcrusher(std::size_t sampleRate, std::size_t numChannels);

		void process(float* pData, std::size_t dataSize);

		virtual void updateStatus(const Status& status) override;
	};
}
