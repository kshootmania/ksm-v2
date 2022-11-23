#include "ksmaudio/stream_with_effects.hpp"
#include <stdexcept>

namespace ksmaudio
{
	StreamWithEffects::StreamWithEffects(const std::string& filePath)
		: m_stream(filePath)
	{
	}

	void StreamWithEffects::play() const
	{
		m_stream.play();
	}

	void StreamWithEffects::pause() const
	{
		m_stream.pause();
	}

	void StreamWithEffects::stop() const
	{
		m_stream.stop();
	}

	double StreamWithEffects::posSec() const
	{
		return m_stream.posSec();
	}

	void StreamWithEffects::seekPosSec(double timeSec) const
	{
		m_stream.seekPosSec(timeSec);
	}

	double StreamWithEffects::durationSec() const
	{
		return m_stream.durationSec();
	}

	std::size_t StreamWithEffects::sampleRate() const
	{
		return m_stream.sampleRate();
	}

	std::size_t StreamWithEffects::numChannels() const
	{
		return m_stream.numChannels();
	}

	double StreamWithEffects::latencySec() const
	{
		return m_stream.latencySec();
	}

	AudioEffect::AudioEffectBus* StreamWithEffects::emplaceAudioEffectBus()
	{
		// Note: It is intentional to return the internal raw pointer of unique_ptr here.
		//       Management of the returned pointer is the responsibility of the caller.
		return m_audioEffectBuses.emplace_back(std::make_unique<AudioEffect::AudioEffectBus>(&m_stream)).get();
	}
}
