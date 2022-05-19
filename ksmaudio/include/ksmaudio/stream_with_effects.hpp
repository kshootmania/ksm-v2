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
		std::unordered_map<std::string, HDSP> m_audioEffectHDSPs;

	public:
		// TODO: filePath encoding problem
		explicit StreamWithEffects(const std::string& filePath);

		~StreamWithEffects();

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
			audioEffect->updateStatus(AudioEffect::Status{});

			m_audioEffects.insert_or_assign(name, std::move(audioEffect));

			const HDSP hDSP = m_stream.addAudioEffect(m_audioEffects.at(name).get(), 0/*FIXME*/);
			if (m_audioEffectHDSPs.contains(name))
			{
				m_stream.removeAudioEffect(m_audioEffectHDSPs.at(name));
			}
			m_audioEffectHDSPs.insert_or_assign(name, hDSP);
		}

		void updateAudioEffectStatus(const AudioEffect::Status& status);
	};
}
