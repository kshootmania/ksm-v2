#include "ksmaudio/audio_effect/dsp/pitch_shift_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	PitchShiftDSP::PitchShiftDSP(const DSPCommonInfo& info)
		: m_info(info)
	{
	}

	void PitchShiftDSP::process(float* pData, std::size_t dataSize, bool bypass, const PitchShiftDSPParams& params)
	{
	}

	void PitchShiftDSP::updateParams(const PitchShiftDSPParams& params)
	{
	}
}