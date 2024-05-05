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

		StreamWithEffects(StreamWithEffects&&) = default;

		StreamWithEffects& operator=(StreamWithEffects&&) = default;

		void play() const;

		void pause() const;

		void stop() const;

		void updateManually() const;

		SecondsF posSec() const;

		void seekPosSec(SecondsF timeSec) const;

		Duration duration() const;

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		Duration latency() const;

		void setFadeOut(Duration duration) const;

		// Note: The pointer is valid until this StreamWithEffects instance is destroyed.
		AudioEffect::AudioEffectBus* emplaceAudioEffectBusFX();
		AudioEffect::AudioEffectBus* emplaceAudioEffectBusLaser();
	};
}
