#pragma once
#include <unordered_map>
#include "audio_effect/audio_effect_bus.hpp"

namespace ksmaudio
{
	class StreamWithEffects
	{
	private:
		Stream m_stream;

		// Note: unique_ptr is employed here because AudioEffectBus cannot be moved (because of const members).
		std::vector<std::unique_ptr<AudioEffect::AudioEffectBus>> m_audioEffectBuses;

		AudioEffect::AudioEffectBus* emplaceAudioEffectBusImpl(bool isLaser);

	public:
		// TODO: filePath encoding problem
		explicit StreamWithEffects(const std::string& filePath, double volume = 1.0, bool enableCompressor = false, bool preload = false);

		StreamWithEffects(const StreamWithEffects&) = delete;

		StreamWithEffects& operator=(const StreamWithEffects&) = delete;

		void play() const;

		void pause() const;

		void stop() const;

		void updateManually() const;

		double posSec() const;

		void seekPosSec(double timeSec) const;

		double durationSec() const;

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		double latencySec() const;

		void setFadeOut(double durationSec) const;

		// Note: The pointer is valid until this StreamWithEffects instance is destroyed.
		AudioEffect::AudioEffectBus* emplaceAudioEffectBusFX();
		AudioEffect::AudioEffectBus* emplaceAudioEffectBusLaser();
	};
}
