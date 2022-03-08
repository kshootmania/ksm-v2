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
		kBitCrusher,
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

	struct AudioEffectParam
	{
		double valueOff = 0.0;
		double valueOnMin = 0.0;
		double valueOnMax = 0.0;
	};

	using AudioEffectParams = std::unordered_map<String, AudioEffectParam>;

	struct AudioEffectDef
	{
		AudioEffectType type = AudioEffectType::kUnspecified;
		AudioEffectParams params;
	};

	struct AudioEffectRoot
	{
		DefList<AudioEffectDef> defList;
		InvokeList<ByPulse<AudioEffectParams>> pulseEventList;
		InvokeList<ByNotes<AudioEffectParams>> noteEventList;
	};
}
