#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct RetriggerEchoDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: 負の値にすると無視される
		bool updateTrigger = false;
		float waveLength = 0.0f;
		float rate = 0.7f;
		bool fadesOut = false;
		float feedbackLevel = 1.0f;
		float mix = 1.0f;
	};

	struct RetriggerParams
	{
		Param updatePeriod = DefineParam(Type::kLength, "1/2");
		Param waveLength = DefineParam(Type::kLength, "0");
		Param rate = DefineParam(Type::kRate, "70%");
		UpdateTriggerParam updateTrigger = DefineUpdateTriggerParam("off");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kUpdatePeriod, &updatePeriod },
			{ ParamID::kWaveLength, &waveLength },
			{ ParamID::kRate, &rate },
			{ ParamID::kUpdateTrigger, updateTrigger.innerParamPtr() },
			{ ParamID::kMix, &mix },
		};

		RetriggerEchoDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.updateTrigger = updateTrigger.render(status, laneIdx),
				.waveLength = GetValue(waveLength, status, isOn),
				.rate = GetValue(rate, status, isOn),
				.fadesOut = false, // Retriggerではfalse固定
				.feedbackLevel = 1.0f, // Retriggerでは1固定
				.mix = GetValue(mix, status, isOn),
				// secUntilTriggerは利用側(BasicAudioEffectWithTrigger::updateStatus())で設定されるのでここでは指定しない
			};
		}
	};

	struct EchoParams
	{
		Param updatePeriod = DefineParam(Type::kLength, "0");
		Param waveLength = DefineParam(Type::kLength, "0");
		UpdateTriggerParam updateTrigger = DefineUpdateTriggerParam("off>on");
		Param feedbackLevel = DefineParam(Type::kRate, "100%");
		Param mix = DefineParam(Type::kRate, "0%>100%");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kUpdatePeriod, &updatePeriod },
			{ ParamID::kWaveLength, &waveLength },
			{ ParamID::kUpdateTrigger, updateTrigger.innerParamPtr() },
			{ ParamID::kFeedbackLevel, &feedbackLevel },
			{ ParamID::kMix, &mix },
		};

		RetriggerEchoDSPParams render(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return {
				.updateTrigger = updateTrigger.render(status, laneIdx),
				.waveLength = GetValue(waveLength, status, isOn),
				.rate = 1.0f, // Echoでは1固定
				.fadesOut = true, // Echoではtrue固定
				.feedbackLevel = GetValue(feedbackLevel, status, isOn),
				.mix = GetValue(mix, status, isOn),
				// secUntilTriggerは利用側(BasicAudioEffectWithTrigger::updateStatus())で設定されるのでここでは指定しない
			};
		}
	};
}
