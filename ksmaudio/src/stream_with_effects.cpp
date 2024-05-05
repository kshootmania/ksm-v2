#include "ksmaudio/stream_with_effects.hpp"

namespace ksmaudio
{
	AudioEffect::AudioEffectBus* StreamWithEffects::emplaceAudioEffectBusImpl(bool isLaser)
	{
		// Note: It is intentional to return the internal raw pointer of unique_ptr here.
		//       Management of the returned pointer is the responsibility of the caller.
		return m_audioEffectBuses.emplace_back(std::make_unique<AudioEffect::AudioEffectBus>(isLaser, &m_stream)).get();
	}

	StreamWithEffects::StreamWithEffects(const std::string& filePath, double volume, bool enableCompressor, bool preload)
		: m_stream(filePath, volume, enableCompressor, preload)
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

	void StreamWithEffects::updateManually() const
	{
		m_stream.updateManually();
	}

	SecondsF StreamWithEffects::posSec() const
	{
		return m_stream.posSec();
	}

	void StreamWithEffects::seekPosSec(SecondsF timeSec) const
	{
		m_stream.seekPosSec(timeSec);
	}

	Duration StreamWithEffects::duration() const
	{
		return m_stream.duration();
	}

	std::size_t StreamWithEffects::sampleRate() const
	{
		return m_stream.sampleRate();
	}

	std::size_t StreamWithEffects::numChannels() const
	{
		return m_stream.numChannels();
	}

	Duration StreamWithEffects::latency() const
	{
		return m_stream.latency();
	}

	void StreamWithEffects::setFadeOut(Duration duration) const
	{
		m_stream.setFadeOut(duration);
	}

	AudioEffect::AudioEffectBus* StreamWithEffects::emplaceAudioEffectBusFX()
	{
		return emplaceAudioEffectBusImpl(false);
	}

	AudioEffect::AudioEffectBus* StreamWithEffects::emplaceAudioEffectBusLaser()
	{
		return emplaceAudioEffectBusImpl(true);
	}
}
