#include "kson/audio/audio_effect.hpp"

namespace
{
	using namespace kson;

	const std::unordered_map<std::string_view, AudioEffectType> s_ksonPresetAudioEffectNames
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
}

kson::AudioEffectType kson::StrToAudioEffectType(std::string_view str)
{
	if (s_ksonPresetAudioEffectNames.contains(str))
	{
		return s_ksonPresetAudioEffectNames.at(str);
	}
	else
	{
		return AudioEffectType::Unspecified;
	}
}

void kson::to_json(nlohmann::json& j, const AudioEffectParams& params)
{
	j = nlohmann::json::object();

	for (const auto& [name, value] : params)
	{
		if (value.value == value.valueOn)
		{
			if (value.valueOn == value.valueOnMax)
			{
				// "xxx"
				j[name] = value.value;
			}
			else
			{
				// "xxx-yyy"
				j[name] = value.value;
				j[name + ".on.max"] = value.valueOnMax;
			}
		}
		else
		{
			if (value.valueOn == value.valueOnMax)
			{
				// "xxx>yyy"
				j[name] = value.value;
				j[name + ".on"] = value.valueOn;
			}
			else
			{
				// "xxx>yyy-zzz"
				j[name] = value.value;
				j[name + ".on"] = value.valueOn;
				j[name + ".on.max"] = value.valueOnMax;
			}
		}
	}
}

void kson::to_json(nlohmann::json& j, const AudioEffectDef& def)
{
	j["type"] = def.type;

	if (!nlohmann::json(def.params).empty())
	{
		j["v"] = def.params;
	}
}

kson::AudioEffectParam::AudioEffectParam(double value)
	: value(value)
	, valueOn(value)
	, valueOnMax(value)
{
}

kson::AudioEffectParam::AudioEffectParam(double value, double valueOn)
	: value(value)
	, valueOn(valueOn)
	, valueOnMax(valueOn)
{
}

kson::AudioEffectParam::AudioEffectParam(double value, double valueOn, double valueOnMax)
	: value(value)
	, valueOn(valueOn)
	, valueOnMax(valueOnMax)
{
}

bool kson::AudioEffectFXInfo::empty() const
{
	return def.empty() && paramChange.empty() && longInvoke.empty();
}

void kson::to_json(nlohmann::json& j, const AudioEffectFXInfo& fx)
{
	j = nlohmann::json::object();

	if (!fx.def.empty())
	{
		j["def"] = fx.def;
	}

	if (!fx.paramChange.empty())
	{
		j["param_change"] = fx.paramChange;
	}

	if (!fx.longInvoke.empty())
	{
		j["long_invoke"] = fx.longInvoke;
	}
}


bool kson::AudioEffectLaserInfo::empty() const
{
	return def.empty() && paramChange.empty() && pulseInvoke.empty();
}

void kson::to_json(nlohmann::json& j, const AudioEffectLaserInfo& laser)
{
	j = nlohmann::json::object();

	if (!laser.def.empty())
	{
		j["def"] = laser.def;
	}

	if (!laser.paramChange.empty())
	{
		j["param_change"] = laser.paramChange;
	}

	if (!laser.pulseInvoke.empty())
	{
		j["pulse_invoke"] = laser.pulseInvoke;
	}
}

bool kson::AudioEffectInfo::empty() const
{
	return fx.empty() && laser.empty();
}

void kson::to_json(nlohmann::json& j, const AudioEffectInfo& audioEffect)
{
	if (!audioEffect.fx.empty())
	{
		j["fx"] = audioEffect.fx;
	}

	if (!audioEffect.laser.empty())
	{
		j["laser"] = audioEffect.laser;
	}
}
