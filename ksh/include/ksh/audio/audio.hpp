#pragma once
#include "ksh/common/common.hpp"
#include "bgm_info.hpp"
#include "key_sound.hpp"
#include "audio_effect.hpp"
#include "legacy_audio_info.hpp"

namespace ksh
{
	struct AudioRoot
	{
		BGMInfo bgmInfo;
		KeySoundRoot keySounds;
		AudioEffectRoot audioEffects;
		LegacyAudioInfo legacy;
	};

	void to_json(nlohmann::json& j, const AudioRoot& audio);

	struct MetaAudioRoot
	{
		MetaBGMInfo bgmInfo;
	};
}
