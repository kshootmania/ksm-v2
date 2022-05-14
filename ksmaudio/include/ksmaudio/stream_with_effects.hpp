#pragma once
#include <memory>
#include <unordered_map>
#include <concepts>
#include "stream.hpp"
#include "audio_effect/audio_effect.hpp"

namespace ksmaudio
{
	class StreamWithEffects
	{
	private:
		Stream m_stream;
		std::unordered_map<std::string, std::unique_ptr<AudioEffect::IAudioEffect>> m_audioEffects;

	public:
		// TODO: filePath encoding problem
		explicit StreamWithEffects(const std::string& filePath);

		void play() const;

		void pause() const;

		void stop() const;

		double posSec() const;

		void seekPosSec(double timeSec) const;

		double durationSec() const;

		std::size_t sampleRate() const;

		std::size_t numChannels() const;

		template <typename T>
		void emplaceAudioEffect(const std::string& name, const std::unordered_map<std::string, std::string>& params = {}) requires std::derived_from<T, AudioEffect::IAudioEffect>
		{
			auto audioEffect = std::make_unique<T>(sampleRate(), numChannels());

			for (const auto& [paramName, valueSetStr] : params)
			{
				audioEffect->setParamValueSet(paramName, valueSetStr);
			}

			m_audioEffects.insert_or_assign(name, std::move(audioEffect));

			m_stream.addAudioEffect(m_audioEffects.at(name).get(), 0/*FIXME*/);
		}

		void updateAudioEffectStatus(const AudioEffect::Status& status);
	};
}
