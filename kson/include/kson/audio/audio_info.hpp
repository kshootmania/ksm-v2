#pragma once
#include "kson/common/common.hpp"
#include "bgm_info.hpp"
#include "key_sound.hpp"
#include "audio_effect.hpp"

namespace kson
{
	struct AudioInfo
	{
		BGMInfo bgm;
		KeySoundInfo keySound;
		AudioEffectInfo audioEffect;
	};

	struct MetaAudioInfo
	{
		MetaBGMInfo bgm;
	};
}
