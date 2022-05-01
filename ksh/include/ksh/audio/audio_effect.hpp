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
		AudioEffectParam(double value);

		AudioEffectParam(double valueOff, double valueOn);

		AudioEffectParam(double valueOff, double valueOnMin, double valueOnMax);
	};

	using AudioEffectParams = std::unordered_map<std::string, AudioEffectParam>;

	void to_json(nlohmann::json& j, const AudioEffectParams& params);

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::kUnspecified;
		AudioEffectParams params;
	};

	void to_json(nlohmann::json& j, const AudioEffectDef& def);

	struct AudioEffectRoot
	{
		DefList<AudioEffectDef> defList;
		InvokeList<ByPulse<AudioEffectParams>> pulseEventList;
		InvokeList<ByNotes<AudioEffectParams>> noteEventList;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const AudioEffectRoot& audioEffect);
}
