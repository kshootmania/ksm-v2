#include "kson/audio/audio_info.hpp"

void kson::to_json(nlohmann::json& j, const AudioInfo& audio)
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
