#pragma once
#include "audio_effect.hpp"

#include "dsp/retrigger_echo_dsp.hpp"
#include "params/retrigger_echo_params.hpp"

#include "dsp/gate_dsp.hpp"
#include "params/gate_params.hpp"

#include "dsp/flanger_dsp.hpp"
#include "params/flanger_params.hpp"

#include "dsp/bitcrusher_dsp.hpp"
#include "params/bitcrusher_params.hpp"

#include "dsp/wobble_dsp.hpp"
#include "params/wobble_params.hpp"

#include "dsp/tapestop_dsp.hpp"
#include "params/tapestop_params.hpp"

namespace ksmaudio
{
	using Retrigger = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::RetriggerParams, AudioEffect::RetriggerEchoDSP, AudioEffect::RetriggerEchoDSPParams>;

	using Gate = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::GateParams, AudioEffect::GateDSP, AudioEffect::GateDSPParams>;

	using Flanger = AudioEffect::BasicAudioEffect<AudioEffect::FlangerParams, AudioEffect::FlangerDSP, AudioEffect::FlangerDSPParams>;

	using Bitcrusher = AudioEffect::BasicAudioEffect<AudioEffect::BitcrusherParams, AudioEffect::BitcrusherDSP, AudioEffect::BitcrusherDSPParams>;

	using Wobble = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::WobbleParams, AudioEffect::WobbleDSP, AudioEffect::WobbleDSPParams>;

	using Tapestop = AudioEffect::BasicAudioEffect<AudioEffect::TapestopParams, AudioEffect::TapestopDSP, AudioEffect::TapestopDSPParams>;

	using Echo = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::EchoParams, AudioEffect::RetriggerEchoDSP, AudioEffect::RetriggerEchoDSPParams>;
}
