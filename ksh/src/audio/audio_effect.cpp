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

void ksh::to_json(nlohmann::json& j, const AudioEffectParams& params)
{
	j = nlohmann::json::object();
	for (const auto& [name, value] : params)
	{
		if (value.valueOff == value.valueOnMin)
		{
			if (value.valueOnMin == value.valueOnMax)
			{
				// "xxx"
				j[name] = value.valueOff;
			}
			else
			{
				// "xxx-yyy"
				j[name] = value.valueOff;
				j[name + ".on.max"] = value.valueOnMax;
			}
		}
		else
		{
			if (value.valueOnMin == value.valueOnMax)
			{
				// "xxx>yyy"
				j[name] = value.valueOff;
				j[name + ".on"] = value.valueOnMin;
			}
			else
			{
				// "xxx>yyy-zzz"
				j[name] = value.valueOff;
				j[name + ".on.min"] = value.valueOnMin;
				j[name + ".on.max"] = value.valueOnMax;
			}
		}
	}
}

void ksh::to_json(nlohmann::json& j, const AudioEffectDef& def)
{
	j["type"] = def.type;

	if (!nlohmann::json(def.params).empty())
	{
		j["v"] = def.params;
	}
}

ksh::AudioEffectParam::AudioEffectParam(double value)
	: valueOff(value)
	, valueOnMin(value)
	, valueOnMax(value)
{
}

ksh::AudioEffectParam::AudioEffectParam(double valueOff, double valueOn)
	: valueOff(valueOff)
	, valueOnMin(valueOn)
	, valueOnMax(valueOn)
{
}

ksh::AudioEffectParam::AudioEffectParam(double valueOff, double valueOnMin, double valueOnMax)
	: valueOff(valueOff)
	, valueOnMin(valueOnMin)
	, valueOnMax(valueOnMax)
{
}

bool ksh::AudioEffectRoot::empty() const
{
	return defList.empty() && pulseEventList.empty() && noteEventList.empty();
}


void ksh::to_json(nlohmann::json& j, const AudioEffectRoot& audioEffect)
{
	j = nlohmann::json::object();

	if (!audioEffect.defList.empty())
	{
		j["def"] = audioEffect.defList;
	}

	if (!audioEffect.pulseEventList.empty())
	{
		j["pulse_event"] = audioEffect.pulseEventList;
	}

	if (!audioEffect.noteEventList.empty())
	{
		j["note_event"] = audioEffect.noteEventList;
	}
}