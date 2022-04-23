#include "ksh/audio/audio_effect.hpp"

namespace
{
	using namespace ksh;

	const std::unordered_map<std::string_view, AudioEffectType> s_ksonPresetAudioEffectNames
	{
		{ "retrigger", AudioEffectType::kRetrigger },
		{ "gate", AudioEffectType::kGate },
		{ "flanger", AudioEffectType::kFlanger },
		{ "pitch_shift", AudioEffectType::kPitchShift },
		{ "bitcrusher", AudioEffectType::kBitcrusher },
		{ "phaser", AudioEffectType::kPhaser },
		{ "wobble", AudioEffectType::kWobble },
		{ "tapestop", AudioEffectType::kTapestop },
		{ "echo", AudioEffectType::kEcho },
		{ "sidechain", AudioEffectType::kSidechain },
		{ "audio_swap", AudioEffectType::kAudioSwap },
		{ "high_pass_filter", AudioEffectType::kAudioSwap },
		{ "low_pass_filter", AudioEffectType::kAudioSwap },
		{ "peaking_filter", AudioEffectType::kAudioSwap },
	};
}

ksh::AudioEffectType ksh::StrToAudioEffectType(std::string_view str)
{
	if (s_ksonPresetAudioEffectNames.contains(str))
	{
		return s_ksonPresetAudioEffectNames.at(str);
	}
	else
	{
		return AudioEffectType::kUnspecified;
	}
}
