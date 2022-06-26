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

	std::string_view AudioEffectTypeToStr(AudioEffectType type);

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
	};

	struct AudioEffectLaserInfo
	{
		Dict<AudioEffectDef> def;
		Dict<Dict<ByPulse<std::string>>> paramChange;
		Dict<std::set<Pulse>> pulseEvent;
		std::int32_t peakingFilterDelay = 0; // 0ms - 160ms
	};

	struct AudioEffectInfo
	{
		AudioEffectFXInfo fx;
		AudioEffectLaserInfo laser;
	};
}
