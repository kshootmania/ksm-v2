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

	inline void to_json(nlohmann::json& j, const AudioRoot& audio)
	{
		j["bgm"] = audio.bgmInfo;

		if (!audio.audioEffects.empty())
		{
			j["audio_effect"] = audio.audioEffects;
		}

		if (!audio.keySounds.empty())
		{
			j["key_sound"] = audio.keySounds;
		}

		if (!audio.legacy.empty())
		{
			j["legacy"] = audio.legacy;
		}
	}

	/*inline void from_json(const nlohmann::json& j, AudioRoot& audio)
	{
		j.at("bgm").get_to(audio.bgmInfo);
		j.at("key_sound").get_to(audio.keySounds);
		j.at("audio_effect").get_to(audio.audioEffects);
	}*/
}
