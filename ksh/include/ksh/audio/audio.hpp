#pragma once
#include "ksh/common/common.hpp"
#include "bgm_info.hpp"
#include "key_sound.hpp"
#include "audio_effect.hpp"

namespace ksh
{
	struct AudioRoot
	{
		BGMInfo bgmInfo;
		KeySoundRoot keySounds;
		AudioEffectRoot audioEffects;
	};
}
