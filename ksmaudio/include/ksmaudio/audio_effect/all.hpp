#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"

#include "ksmaudio/audio_effect/dsp/flanger_dsp.hpp"
#include "ksmaudio/audio_effect/params/flanger_params.hpp"

#include "ksmaudio/audio_effect/dsp/bitcrusher_dsp.hpp"
#include "ksmaudio/audio_effect/params/bitcrusher_params.hpp"

namespace ksmaudio::AudioEffect
{
	using Flanger = BasicAudioEffect<FlangerParams, FlangerDSP, FlangerDSPParams>;

	using Bitcrusher = BasicAudioEffect<BitcrusherParams, BitcrusherDSP, BitcrusherDSPParams>;
}
