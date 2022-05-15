#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"

#include "ksmaudio/audio_effect/dsp/flanger_dsp.hpp"
#include "ksmaudio/audio_effect/params/flanger_params.hpp"

#include "ksmaudio/audio_effect/dsp/bitcrusher_dsp.hpp"
#include "ksmaudio/audio_effect/params/bitcrusher_params.hpp"

namespace ksmaudio
{
	using Flanger = AudioEffect::BasicAudioEffect<AudioEffect::FlangerParams, AudioEffect::FlangerDSP, AudioEffect::FlangerDSPParams>;

	using Bitcrusher = AudioEffect::BasicAudioEffect<AudioEffect::BitcrusherParams, AudioEffect::BitcrusherDSP, AudioEffect::BitcrusherDSPParams>;
}