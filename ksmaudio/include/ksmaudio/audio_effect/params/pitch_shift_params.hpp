#pragma once
#include <unordered_map>
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct PitchShiftDSPParams
	{
		float pitch = 10.0f;
		float chunkSize = 700.0f;
		float overWrap = 40.0f;
		float mix = 1.0f;
	};

	struct PitchShiftParams
	{
		Param pitch = DefineParam(Type::kPitch, "16");
		Param chunkSize = DefineParam(Type::kSample, "700samples");
		Param overWrap = DefineParam(Type::kRate, "40%");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kPitch, &pitch },
			{ ParamID::kChunkSize, &chunkSize },
			{ ParamID::kOverWrap, &overWrap },
			{ ParamID::kMix, &mix },
		};

		PitchShiftDSPParams render(const Status& status, bool isOn)
		{
			return {
				.pitch = GetValue(pitch, status, isOn),
				.chunkSize = GetValue(chunkSize, status, isOn),
				.overWrap = GetValue(overWrap, status, isOn),
				.mix = GetValue(mix, status, isOn),
			};
		}

		PitchShiftDSPParams renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return render(status, isOn);
		}

		PitchShiftDSPParams renderByLaser(const Status& status, bool isOn)
		{
			return render(status, isOn);
		}
	};
};