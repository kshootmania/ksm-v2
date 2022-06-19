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

	using AudioEffectParams = Dict<std::string>;

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::Unspecified;
		AudioEffectParams v;
	};

	struct AudioEffectFXInfo
	{
		Dict<AudioEffectDef> def;
		Dict<Dict<ByPulse<std::string>>> paramChange;
		Dict<FXLane<AudioEffectParams>> longEvent;

		bool empty() const;
	};

	struct AudioEffectLaserInfo
	{
		Dict<AudioEffectDef> def;
		Dict<Dict<ByPulse<std::string>>> paramChange;
		Dict<ByPulse<AudioEffectParams>> pulseEvent;

		bool empty() const;
	};

	struct AudioEffectInfo
	{
		AudioEffectFXInfo fx;
		AudioEffectLaserInfo laser;

		bool empty() const;
	};
}
