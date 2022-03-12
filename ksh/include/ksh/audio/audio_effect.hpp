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

	using AudioEffectParams = std::unordered_map<std::u8string, AudioEffectParam>;

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
