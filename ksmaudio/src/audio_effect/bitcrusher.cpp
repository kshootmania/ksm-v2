#include "ksmaudio/audio_effect/bitcrusher.hpp"
#include "ksmaudio/audio_effect/dsp/bitcrusher_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	Bitcrusher::Bitcrusher(std::size_t sampleRate, std::size_t numChannels)
		: m_dsp(std::make_unique<BitcrusherDSP>(DSPCommonInfo{ sampleRate, numChannels }))
	{
	}

	void Bitcrusher::process(float* pData, std::size_t dataSize)
	{
		m_dsp->process(pData, dataSize, m_bypass, m_dspParams);
	}

	void Bitcrusher::updateStatus(const Status& status)
	{
		m_dspParams = m_params.render(status);
	}
}
