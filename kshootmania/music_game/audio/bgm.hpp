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
		Stopwatch m_manualUpdateStopwatch;

		void emplaceAudioEffectImpl(
			bool isFX,
			const std::string& name,
			const kson::AudioEffectDef& def,
			const std::set<float>& updateTriggerTiming);

	public:
		explicit BGM(FilePathView filePath);

		void update();

		void updateAudioEffectFX(bool bypass, const ksmaudio::AudioEffect::Status& status, const ksmaudio::AudioEffect::ActiveAudioEffectDict& activeAudioEffects);

		// Siv3D非依存のksmaudioに渡す必要があるため、ここではOptionalではなくstd::optionalを使用(これらは暗黙変換されない)
		void updateAudioEffectLaser(bool bypass, const ksmaudio::AudioEffect::Status& status, const std::optional<std::size_t>& activeAudioEffectIdx);

		void play();

		void pause();

		void seekPosSec(double posSec);

		double posSec() const;

		double durationSec() const;

		double latencySec() const;

		void emplaceAudioEffectFX(
			const std::string& name,
			const kson::AudioEffectDef& def,
			const std::set<float>& updateTriggerTiming = {}); // TODO: param_change

		void emplaceAudioEffectLaser(
			const std::string& name,
			const kson::AudioEffectDef& def,
			const std::set<float>& updateTriggerTiming = {}); // TODO: param_change

		const ksmaudio::AudioEffect::AudioEffectBus& audioEffectBusFX() const;

		const ksmaudio::AudioEffect::AudioEffectBus& audioEffectBusLaser() const;
	};
}
