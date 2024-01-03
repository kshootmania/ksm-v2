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

#include "dsp/phaser_dsp.hpp"
#include "params/phaser_params.hpp"

#include "dsp/wobble_dsp.hpp"
#include "params/wobble_params.hpp"

#include "dsp/tapestop_dsp.hpp"
#include "params/tapestop_params.hpp"

#include "dsp/sidechain_dsp.hpp"
#include "params/sidechain_params.hpp"

#include "dsp/peaking_filter_dsp.hpp"
#include "params/peaking_filter_params.hpp"

#include "dsp/high_pass_filter_dsp.hpp"
#include "params/high_pass_filter_params.hpp"

#include "dsp/low_pass_filter_dsp.hpp"
#include "params/low_pass_filter_params.hpp"

namespace ksmaudio
{
	// priorityはHSP版と同一に設定
	// HSP版: https://github.com/kshootmania/ksm-v1/blob/ea05374a3ece796612b29d927cb3c6f5aabb266e/src/audio/fxdef.hsp
	// 別途定義されているコンプレッサーの優先度(kCompressorFXPriority)を下回らないよう注意

	using Retrigger = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::RetriggerParams, AudioEffect::RetriggerEchoDSP, AudioEffect::RetriggerEchoDSPParams, 25>;

	using Gate = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::GateParams, AudioEffect::GateDSP, AudioEffect::GateDSPParams, 17>;

	using Flanger = AudioEffect::BasicAudioEffect<AudioEffect::FlangerParams, AudioEffect::FlangerDSP, AudioEffect::FlangerDSPParams, 15>;

	using Bitcrusher = AudioEffect::BasicAudioEffect<AudioEffect::BitcrusherParams, AudioEffect::BitcrusherDSP, AudioEffect::BitcrusherDSPParams, 20>;

	using Phaser = AudioEffect::BasicAudioEffect<AudioEffect::PhaserParams, AudioEffect::PhaserDSP, AudioEffect::PhaserDSPParams, 16>;

	using Wobble = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::WobbleParams, AudioEffect::WobbleDSP, AudioEffect::WobbleDSPParams, 17>;

	using Tapestop = AudioEffect::BasicAudioEffect<AudioEffect::TapestopParams, AudioEffect::TapestopDSP, AudioEffect::TapestopDSPParams, 18>;

	using Echo = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::EchoParams, AudioEffect::RetriggerEchoDSP, AudioEffect::RetriggerEchoDSPParams, 26>;

	using Sidechain = AudioEffect::BasicAudioEffectWithTrigger<AudioEffect::SidechainParams, AudioEffect::SidechainDSP, AudioEffect::SidechainDSPParams, 14>;

	using PeakingFilter = AudioEffect::BasicAudioEffect<AudioEffect::PeakingFilterParams, AudioEffect::PeakingFilterDSP, AudioEffect::PeakingFilterDSPParams, 1>;

	using HighPassFilter = AudioEffect::BasicAudioEffect<AudioEffect::HighPassFilterParams, AudioEffect::HighPassFilterDSP, AudioEffect::HighPassFilterDSPParams, 2>;

	using LowPassFilter = AudioEffect::BasicAudioEffect<AudioEffect::LowPassFilterParams, AudioEffect::LowPassFilterDSP, AudioEffect::LowPassFilterDSPParams, 3>;
}
