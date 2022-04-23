#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	enum class AudioEffectType
	{
		kUnspecified,
		kRetrigger,
		kGate,
		kFlanger,
		kPitchShift,
		kBitcrusher,
		kPhaser,
		kWobble,
		kTapestop,
		kEcho,
		kSidechain,
		kAudioSwap,
		kHighPassFilter,
		kLowPassFilter,
		kPeakingFilter,
	};

	AudioEffectType StrToAudioEffectType(std::string_view str);

	NLOHMANN_JSON_SERIALIZE_ENUM(AudioEffectType, {
		{ AudioEffectType::kUnspecified, nullptr },
		{ AudioEffectType::kRetrigger, "retrigger" },
		{ AudioEffectType::kGate, "gate" },
		{ AudioEffectType::kFlanger, "flanger" },
		{ AudioEffectType::kPitchShift, "pitch_shift" },
		{ AudioEffectType::kBitcrusher, "bitcrusher" },
		{ AudioEffectType::kPhaser, "phaser" },
		{ AudioEffectType::kWobble, "wobble" },
		{ AudioEffectType::kTapestop, "tapestop" },
		{ AudioEffectType::kEcho, "echo" },
		{ AudioEffectType::kSidechain, "sidechain" },
		{ AudioEffectType::kAudioSwap, "audio_swap" },
		{ AudioEffectType::kHighPassFilter, "high_pass_filter" },
		{ AudioEffectType::kLowPassFilter, "low_pass_filter" },
		{ AudioEffectType::kPeakingFilter, "peaking_filter" },
	});

	struct AudioEffectParam
	{
		double valueOff = 0.0;
		double valueOnMin = 0.0;
		double valueOnMax = 0.0;

		AudioEffectParam() = default;

		// Note: Implicit conversion from double to AudioEffectParam is allowed
		AudioEffectParam(double value)
			: valueOff(value)
			, valueOnMin(value)
			, valueOnMax(value)
		{
		}

		AudioEffectParam(double valueOff, double valueOn)
			: valueOff(valueOff)
			, valueOnMin(valueOn)
			, valueOnMax(valueOn)
		{
		}

		AudioEffectParam(double valueOff, double valueOnMin, double valueOnMax)
			: valueOff(valueOff)
			, valueOnMin(valueOnMin)
			, valueOnMax(valueOnMax)
		{
		}
	};

	using AudioEffectParams = std::unordered_map<std::string, AudioEffectParam>;

	inline void to_json(nlohmann::json& j, const AudioEffectParams& params)
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

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::kUnspecified;
		AudioEffectParams params;
	};

	inline void to_json(nlohmann::json& j, const AudioEffectDef& def)
	{
		j["type"] = def.type;

		if (!nlohmann::json(def.params).empty())
		{
			j["v"] = def.params;
		}
	}

	struct AudioEffectRoot
	{
		DefList<AudioEffectDef> defList;
		InvokeList<ByPulse<AudioEffectParams>> pulseEventList;
		InvokeList<ByNotes<AudioEffectParams>> noteEventList;

		bool empty() const
		{
			return defList.empty() && pulseEventList.empty() && noteEventList.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const AudioEffectRoot& audioEffect)
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
}
