#pragma once
#include <set>
#include "kson/common/common.hpp"

namespace kson
{
	struct KeySoundInvokeFX
	{
		double vol = 1.0;
	};

	// Dictionary key: filename/clap/clap_impact/clap_punchy/snare/snare_lo
	using KeySoundInvokeListFX = Dict<FXLane<KeySoundInvokeFX>>;

	struct KeySoundFXInfo
	{
		KeySoundInvokeListFX chipEvent;
	};

	// Dictionary key: slam_up/slam_down/slam_swing/slam_mute
	using KeySoundInvokeListLaser = Dict<std::set<Pulse>>;

	struct KeySoundLaserLegacyInfo
	{
		bool autoVol = false;
	};

	struct KeySoundLaserInfo
	{
		ByPulse<double> vol;
		KeySoundInvokeListLaser slamEvent;
		KeySoundLaserLegacyInfo legacy;
	};

	struct KeySoundInfo
	{
		KeySoundFXInfo fx;
		KeySoundLaserInfo laser;
	};
}
