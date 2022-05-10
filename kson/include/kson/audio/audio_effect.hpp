#pragma once
#include "kson/common/common.hpp"

namespace kson
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
		double value = 0.0;
		double valueOn = 0.0;
		double valueOnMax = 0.0;

		AudioEffectParam() = default;

		// Note: Implicit conversion from double to AudioEffectParam is allowed
		AudioEffectParam(double value);

		AudioEffectParam(double value, double valueOn);

		AudioEffectParam(double value, double valueOn, double valueOnMax);
	};

	using AudioEffectParams = std::unordered_map<std::string, AudioEffectParam>;

	void to_json(nlohmann::json& j, const AudioEffectParams& params);

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::kUnspecified;
		AudioEffectParams params;
	};

	void to_json(nlohmann::json& j, const AudioEffectDef& def);

	struct AudioEffectFXInfo
	{
		DefList<AudioEffectDef> def;
		InvokeList<ByPulse<AudioEffectParams>> paramChange;
		InvokeList<std::array<ByPulse<AudioEffectParams>, kNumFXLanes>> longInvoke;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const AudioEffectFXInfo& audioEffect);

	struct AudioEffectLaserInfo
	{
		DefList<AudioEffectDef> def;
		InvokeList<ByPulse<AudioEffectParams>> paramChange;
		InvokeList<ByPulse<AudioEffectParams>> pulseInvoke;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const AudioEffectLaserInfo& audioEffect);

	struct AudioEffectRoot
	{
		AudioEffectFXInfo fx;
		AudioEffectLaserInfo laser;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const AudioEffectRoot& audioEffect);
}
