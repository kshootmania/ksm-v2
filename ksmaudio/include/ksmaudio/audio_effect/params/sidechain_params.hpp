#pragma once
#include "ksmaudio/audio_effect/audio_effect_param.hpp"

namespace ksmaudio::AudioEffect
{
	struct SidechainDSPParams
	{
		float secUntilTrigger = -1.0f; // Note: Negative value will be just ignored
		float attackTime = 0.01f;
		float holdTime = 0.05f;
		float releaseTime = 0.125f;
		float ratio = 1.0f;
	};

	struct SidechainParams
	{
		Param period = DefineParam(Type::kLength, "1/4");
		Param attackTime = DefineParam(Type::kLength, "10ms");
		Param holdTime = DefineParam(Type::kLength, "50ms");
		Param releaseTime = DefineParam(Type::kLength, "1/16");
		Param ratio = DefineParam(Type::kFloat, "1.0>5.0");

		const std::unordered_map<ParamID, Param*> dict = {
			{ ParamID::kPeriod, &period },
			{ ParamID::kAttackTime, &attackTime },
			{ ParamID::kHoldTime, &holdTime },
			{ ParamID::kReleaseTime, &releaseTime },
			{ ParamID::kRatio, &ratio },
		};

		SidechainDSPParams render(const Status& status, bool isOn)
		{
			return {
				.attackTime = GetValue(attackTime, status, isOn),
				.holdTime = GetValue(holdTime, status, isOn),
				.releaseTime = GetValue(releaseTime, status, isOn),
				.ratio = GetValue(ratio, status, isOn),
				// secUntilTriggerは利用側(BasicAudioEffectWithTrigger::updateStatus())で設定されるのでここでは指定しない
			};
		}

		SidechainDSPParams renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();
			return render(status, isOn);
		}

		SidechainDSPParams renderByLaser(const Status& status, bool isOn)
		{
			return render(status, isOn);
		}
	};
}
