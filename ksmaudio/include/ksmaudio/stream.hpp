#pragma once
#include <string>
#include "bass.h"
#include "ksmaudio/audio_effect/audio_effect.hpp"

namespace ksmaudio
{
	class Stream
	{
	private:
		const HSTREAM m_hStream;
		const BASS_CHANNELINFO m_info;

	public:
		// TODO: filePath encoding problem
		explicit Stream(const std::string& filePath);

		~Stream();

		void play() const;

		void pause() const;

		void stop() const;

		double posSec() const;

		void seekPosSec(double timeSec) const;

		double durationSec() const;

		HDSP addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const;

		void removeAudioEffect(HDSP hDSP) const;

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		double latencySec() const;
	};
}
