#pragma once
#include "ksmaudio/ksmaudio.hpp"
#include "kson/audio/audio_effect.hpp"

namespace MusicGame::Audio
{
	class BGM
	{
	private:
		ksmaudio::StreamWithEffects m_stream;
		const double m_durationSec;
		double m_timeSec = 0.0;
		bool m_isStreamStarted = false;
		bool m_isPaused = true;
		ksmaudio::AudioEffect::AudioEffectBus* const m_pAudioEffectBusFX;
		ksmaudio::AudioEffect::AudioEffectBus* const m_pAudioEffectBusLaser;
		Stopwatch m_stopwatch;

	public:
		explicit BGM(FilePathView filePath);

		void update();

		void updateAudioEffectFX(const ksmaudio::AudioEffect::Status& status, const std::array<Optional<std::string>, kson::kNumFXLanes>& laneAudioEffectNames);

		void play();

		void pause();

		void seekPosSec(double posSec);

		double posSec() const;

		double durationSec() const;

		void emplaceAudioEffect(
			bool isFX,
			const std::string& name,
			const kson::AudioEffectDef& def,
			const std::set<float>& updateTriggerTiming = {});
	};
}
