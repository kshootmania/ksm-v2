#include "ksmaudio/audio_effect/flanger.hpp"
#include <array>
#include "ksmaudio/audio_effect/detail/ring_buffer.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"

namespace ksmaudio
{

	class FlangerDSP
	{
	private:
		const std::size_t m_sampleRate;

		const float m_sampleRateScale;

		const std::size_t m_numChannels;

		const bool m_isUnsupported;

		RingBuffer<float> m_ringBuffer;

		float m_lfoTimeRate = 0.0f;

		std::array<BiquadFilter<float>, 2> m_lowShelfFilters;

	public:
		FlangerDSP(std::size_t sampleRate, std::size_t numChannels);

		void process(float* pData, std::size_t dataSize, bool bypass, const FlangerParams& params);
	};

	FlangerDSP::FlangerDSP(std::size_t sampleRate, std::size_t numChannels)
		: m_sampleRate(sampleRate)
		, m_sampleRateScale(static_cast<float>(sampleRate) / 44100.0f)
		, m_numChannels(numChannels)
		, m_isUnsupported(m_numChannels == 0U || m_numChannels >= 3U) // Supports stereo and mono only
		, m_ringBuffer(
			static_cast<std::size_t>(sampleRate) * 3 * numChannels, // 3 seconds
			numChannels)
	{
		for (int i = 0; i < 2; ++i)
		{
			m_lowShelfFilters[i].setLowShelfFilter(250.0f, 0.5f, -20.0f, static_cast<float>(m_sampleRate));
		}
	}

	void FlangerDSP::process(float* pData, std::size_t dataSize, bool bypass, const FlangerParams& params)
	{
		if (m_isUnsupported || dataSize > m_ringBuffer.size())
		{
			return;
		}

		assert(dataSize % m_numChannels == 0);
		const std::size_t numFrames = dataSize / m_numChannels;
		if (bypass)
		{
			m_ringBuffer.write(pData, dataSize);
			m_ringBuffer.advanceCursor(numFrames);
			return;
		}

		const float lfoSpeed = 1.0f / params.periodSec / m_sampleRate;
		float* pCursor = pData;
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			for (std::size_t channel = 0; channel < m_numChannels; ++channel)
			{
				const float lfoValue = (channel == 0U) ? Triangle(m_lfoTimeRate) : Triangle(DecimalPart(m_lfoTimeRate + params.stereoWidth / 2));
				const float delayFrames = (params.delay + lfoValue * params.depth) * m_sampleRateScale;
				const float wet = (*pCursor + m_lowShelfFilters[channel].process(m_ringBuffer.lerpedDelay(delayFrames, channel))) * params.vol;
				m_ringBuffer.write(Lerp(*pCursor, wet, params.feedback), channel);
				*pCursor = Lerp(*pCursor, wet, params.mix);
				++pCursor;
			}
			m_ringBuffer.advanceCursor();

			m_lfoTimeRate += lfoSpeed;
			if (m_lfoTimeRate > 1.0f)
			{
				m_lfoTimeRate = DecimalPart(m_lfoTimeRate);
			}
		}
	}

	Flanger::Flanger(std::size_t sampleRate, std::size_t numChannels)
		: m_dsp(std::make_unique<FlangerDSP>(sampleRate, numChannels))
	{
	}

	Flanger::~Flanger() = default;

	void Flanger::process(float* pData, std::size_t dataSize)
	{
		m_dsp->process(pData, dataSize, m_bypass, m_params);
	}

}
