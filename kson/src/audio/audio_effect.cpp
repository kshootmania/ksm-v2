#include "kson/audio/audio_effect.hpp"

namespace
{
	using namespace kson;

	const std::unordered_map<std::string_view, AudioEffectType> s_strToAudioEffectType
	{
		{ "retrigger", AudioEffectType::Retrigger },
		{ "gate", AudioEffectType::Gate },
		{ "flanger", AudioEffectType::Flanger },
		{ "pitch_shift", AudioEffectType::PitchShift },
		{ "bitcrusher", AudioEffectType::Bitcrusher },
		{ "phaser", AudioEffectType::Phaser },
		{ "wobble", AudioEffectType::Wobble },
		{ "tapestop", AudioEffectType::Tapestop },
		{ "echo", AudioEffectType::Echo },
		{ "sidechain", AudioEffectType::Sidechain },
		{ "audio_swap", AudioEffectType::SwitchAudio },
		{ "high_pass_filter", AudioEffectType::HighPassFilter },
		{ "low_pass_filter", AudioEffectType::LowPassFilter },
		{ "peaking_filter", AudioEffectType::PeakingFilter },
	};

	const std::unordered_map<AudioEffectType, std::string_view> s_audioEffectTypeToStr
	{
		{ AudioEffectType::Retrigger, "retrigger" },
		{ AudioEffectType::Gate, "gate" },
		{ AudioEffectType::Flanger, "flanger" },
		{ AudioEffectType::PitchShift, "pitch_shift" },
		{ AudioEffectType::Bitcrusher, "bitcrusher" },
		{ AudioEffectType::Phaser, "phaser" },
		{ AudioEffectType::Wobble, "wobble" },
		{ AudioEffectType::Tapestop, "tapestop" },
		{ AudioEffectType::Echo, "echo" },
		{ AudioEffectType::Sidechain, "sidechain" },
		{ AudioEffectType::SwitchAudio, "audio_swap" },
		{ AudioEffectType::HighPassFilter, "high_pass_filter" },
		{ AudioEffectType::LowPassFilter, "low_pass_filter" },
		{ AudioEffectType::PeakingFilter, "peaking_filter" },
	};
}

kson::AudioEffectType kson::StrToAudioEffectType(std::string_view str)
{
	if (s_strToAudioEffectType.contains(str))
	{
		return s_strToAudioEffectType.at(str);
	}
	else
	{
		return AudioEffectType::Unspecified;
	}
}

std::string_view kson::AudioEffectTypeToStr(AudioEffectType type)
{
	if (s_audioEffectTypeToStr.contains(type))
	{
		return s_audioEffectTypeToStr.at(type);
	}
	else
	{
		return "";
	}
}
