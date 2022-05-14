#include "ksmaudio/audio_effect/flanger.hpp"
#include "ksmaudio/audio_effect/detail/ring_buffer.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/dsp/flanger_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	Flanger::Flanger(std::size_t sampleRate, std::size_t numChannels)
		: m_dsp(std::make_unique<FlangerDSP>(DSPCommonInfo{ sampleRate, numChannels }))
	{
	}

	Flanger::~Flanger() = default;

	void Flanger::process(float* pData, std::size_t dataSize)
	{
		m_dsp->process(pData, dataSize, m_bypass, m_dspParams);
	}
}
