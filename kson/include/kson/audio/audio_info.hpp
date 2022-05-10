#pragma once
#include "kson/common/common.hpp"
#include "bgm_info.hpp"
#include "key_sound.hpp"
#include "audio_effect.hpp"
#include "legacy_audio_info.hpp"

namespace kson
{
	struct AudioInfo
	{
		BGMInfo bgm;
		KeySoundInfo keySounds;
		AudioEffectInfo audioEffects;
		LegacyAudioInfo legacy;
	};

	void to_json(nlohmann::json& j, const AudioInfo& audio);

	struct MetaAudioInfo
	{
		MetaBGMInfo bgm;
	};
}
