#pragma once
#include <unordered_map>
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
		Param period = DefineParam(Type::kLength, "2.0");
		Param delay = DefineParam(Type::kSample, "30samples");
		Param depth = DefineParam(Type::kSample, "45samples");
		Param feedback = DefineParam(Type::kRate, "60%");
		Param stereoWidth = DefineParam(Type::kRate, "0%");
		Param vol = DefineParam(Type::kRate, "75%");
		Param mix = DefineParam(Type::kRate, "0%>80%");

		const std::unordered_map<std::string, Param*> dict = {
			{ "period", &period },
			{ "delay", &delay },
			{ "depth", &depth },
			{ "feedback", &feedback },
			{ "stereo_width", &stereoWidth },
			{ "vol", &vol },
			{ "mix", &mix },
		};

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
