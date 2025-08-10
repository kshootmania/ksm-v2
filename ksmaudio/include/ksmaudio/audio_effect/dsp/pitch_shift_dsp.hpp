#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/pitch_shift_params.hpp"
#include "../../third_party/bass_fx/bass_fx.h"

namespace ksmaudio::AudioEffect
{
	class PitchShiftDSP
	{
	private:
		const DSPCommonInfo m_info;

	public:
		explicit PitchShiftDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const PitchShiftDSPParams& params);

		void updateParams(const PitchShiftDSPParams& params);
	};
}