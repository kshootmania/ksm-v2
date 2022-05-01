#include "ksh/audio/audio.hpp"

void ksh::to_json(nlohmann::json& j, const AudioRoot& audio)
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
