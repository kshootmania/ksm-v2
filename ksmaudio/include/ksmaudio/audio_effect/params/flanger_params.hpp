#pragma once
#include <unordered_map>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct FlangerDSPParams
	{
		float period = 4.0f; // 4s = 2 bars at 120 BPM
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

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kPeriod, &period },
			{ ParamID::kDelay, &delay },
			{ ParamID::kDepth, &depth },
			{ ParamID::kFeedback, &feedback },
			{ ParamID::kStereoWidth, &stereoWidth },
			{ ParamID::kVol, &vol },
			{ ParamID::kMix, &mix },
		};

		FlangerDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.period = GetValue(period, status, isOn),
				.delay = GetValue(delay, status, isOn),
				.depth = GetValue(depth, status, isOn),
				.feedback = GetValue(feedback, status, isOn),
				.stereoWidth = GetValue(stereoWidth, status, isOn),
				.vol = GetValue(vol, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}
	};
}
