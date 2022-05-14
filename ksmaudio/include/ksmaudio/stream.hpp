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

	public:
		// Note: filePath must be in UTF-8
		explicit Stream(const std::string& filePath);

		~Stream();

		void play() const;

		void pause() const;

		void stop() const;

		double posSec() const;

		void seekPosSec(double timeSec) const;

		double durationSec() const;

		void addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const;
	};

}
