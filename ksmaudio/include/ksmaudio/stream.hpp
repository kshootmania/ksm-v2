#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "bass.h"
#include "bass_fx.h"
#include "ksmaudio/audio_effect/audio_effect.hpp"

namespace ksmaudio
{
	using SecondsF = std::chrono::duration<double>;
	using Duration = SecondsF;

	class Stream
	{
	private:
		const std::unique_ptr<std::vector<char>> m_preloadedBinary;
		const HSTREAM m_hStream;
		const BASS_CHANNELINFO m_info;
		double m_volume;

	public:
		explicit Stream(const std::string& filePath, double volume = 1.0, bool enableCompressor = false, bool preload = false, bool loop = false);

		~Stream();

		Stream(const Stream&) = delete;

		Stream& operator=(const Stream&) = delete;

		Stream(Stream&&) = default;

		Stream& operator=(Stream&&) = default;

		void play() const;

		void pause() const;

		void stop() const;

		void updateManually() const;

		SecondsF posSec() const;

		void seekPosSec(SecondsF time) const;

		Duration duration() const;

		HDSP addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const;

		void removeAudioEffect(HDSP hDSP) const;

		void setFadeIn(Duration duration) const;

		void setFadeIn(Duration duration, double volume);

		void setFadeOut(Duration duration) const;

		void setFadeOut(Duration duration, double volume);

		bool isPlaying() const;

		bool isFading() const;

		double volume() const;

		void setVolume(double volume);

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		Duration latency() const;

		void lockBegin() const;

		void lockEnd() const;
	};
}
