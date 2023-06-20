#pragma once
#include <memory>
#include <vector>
#include <string>
#include "bass.h"
#include "bass_fx.h"
#include "ksmaudio/audio_effect/audio_effect.hpp"

namespace ksmaudio
{
	class Stream
	{
	private:
		const std::unique_ptr<std::vector<char>> m_preloadedBinary;
		const HSTREAM m_hStream;
		const BASS_CHANNELINFO m_info;
		float m_volume;

	public:
		// TODO: filePath encoding problem
		explicit Stream(const std::string& filePath, double volume = 1.0, bool enableCompressor = false, bool preload = false, bool loop = false);

		~Stream();

		void play() const;

		void pause() const;

		void stop() const;

		void updateManually() const;

		double posSec() const;

		void seekPosSec(double timeSec) const;

		double durationSec() const;

		HDSP addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const;

		void removeAudioEffect(HDSP hDSP) const;

		void setFadeIn(double durationSec) const;

		void setFadeIn(double durationSec, double volume);

		void setFadeOut(double durationSec) const;

		void setFadeOut(double durationSec, double volume);

		bool isFading() const;

		void setVolume(double volume);

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		double latencySec() const;
	};
}
