#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	enum class AudioEffectType
	{
		Unspecified,
		Retrigger,
		Gate,
		Flanger,
		PitchShift,
		Bitcrusher,
		Phaser,
		Wobble,
		Tapestop,
		Echo,
		Sidechain,
		SwitchAudio,
		HighPassFilter,
		LowPassFilter,
		PeakingFilter,
	};

	AudioEffectType StrToAudioEffectType(std::string_view str);

	NLOHMANN_JSON_SERIALIZE_ENUM(AudioEffectType, {
		{ AudioEffectType::Unspecified, nullptr },
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
		{ AudioEffectType::SwitchAudio, "switch_audio" },
		{ AudioEffectType::HighPassFilter, "high_pass_filter" },
		{ AudioEffectType::LowPassFilter, "low_pass_filter" },
		{ AudioEffectType::PeakingFilter, "peaking_filter" },
	});

	using AudioEffectParams = std::unordered_map<std::string, std::string>;

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::Unspecified;
		AudioEffectParams v;
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

	struct AudioEffectInfo
	{
		AudioEffectFXInfo fx;
		AudioEffectLaserInfo laser;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const AudioEffectInfo& audioEffect);
}
