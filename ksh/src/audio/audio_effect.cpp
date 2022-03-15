#include "ksh/audio/audio_effect.hpp"

namespace
{
	using namespace ksh;

	const std::unordered_map<std::u8string_view, AudioEffectType> s_ksonPresetAudioEffectNames
	{
		{ u8"retrigger", AudioEffectType::kRetrigger },
		{ u8"gate", AudioEffectType::kGate },
		{ u8"flanger", AudioEffectType::kFlanger },
		{ u8"pitch_shift", AudioEffectType::kPitchShift },
		{ u8"bitcrusher", AudioEffectType::kBitcrusher },
		{ u8"phaser", AudioEffectType::kPhaser },
		{ u8"wobble", AudioEffectType::kWobble },
		{ u8"tapestop", AudioEffectType::kTapestop },
		{ u8"echo", AudioEffectType::kEcho },
		{ u8"sidechain", AudioEffectType::kSidechain },
		{ u8"audio_swap", AudioEffectType::kAudioSwap },
		{ u8"high_pass_filter", AudioEffectType::kAudioSwap },
		{ u8"low_pass_filter", AudioEffectType::kAudioSwap },
		{ u8"peaking_filter", AudioEffectType::kAudioSwap },
	};
}

ksh::AudioEffectType ksh::StrToAudioEffectType(std::u8string_view str)
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
