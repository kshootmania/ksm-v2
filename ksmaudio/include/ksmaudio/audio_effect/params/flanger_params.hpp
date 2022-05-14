#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct FlangerDSPParams
	{
		float period = 4.0f;
		float delay = 30.0f;
		float depth = 45.0f;
		float feedback = 0.6f;
		float stereoWidth = 0.0f;
		float vol = 0.75f;
		float mix = 0.8f;
	};

	struct FlangerParams
	{
		Param period = DefineParam("period", Type::kLength, "2.0");
		Param delay = DefineParam("delay", Type::kSample, "30samples");
		Param depth = DefineParam("depth", Type::kSample, "45samples");
		Param feedback = DefineParam("feedback", Type::kRate, "60%");
		Param stereoWidth = DefineParam("stereo_width", Type::kRate, "0%");
		Param vol = DefineParam("vol", Type::kRate, "75%");
		Param mix = DefineParam("mix", Type::kRate, "0%>80%");

		FlangerDSPParams render(const Status& status)
		{
			return {
				.period = GetValue(period, status),
				.delay = GetValue(delay, status),
				.depth = GetValue(depth, status),
				.feedback = GetValue(feedback, status),
				.stereoWidth = GetValue(stereoWidth, status),
				.vol = GetValue(vol, status),
				.mix = GetValue(mix, status),
			};
		}
	};
}
