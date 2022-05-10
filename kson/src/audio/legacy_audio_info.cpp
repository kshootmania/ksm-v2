#include "kson/audio/legacy_audio_info.hpp"

bool kson::LegacyAudioBGMInfo::empty() const
{
	return filenameF.empty() && filenameP.empty() && filenameFP.empty();
}

std::vector<std::string> kson::LegacyAudioBGMInfo::toStrArray() const
{
	if (!filenameFP.empty())
	{
		return {
			filenameF,
			filenameP,
			filenameFP,
		};
	}
	else if (!filenameP.empty())
	{
		return {
			filenameF,
			filenameP,
		};
	}
	else if (!filenameF.empty())
	{
		return {
			filenameF,
		};
	}
	else
	{
		return std::vector<std::string>();
	}
}

void kson::to_json(nlohmann::json& j, const LegacyAudioBGMInfo& legacy)
{
	j = nlohmann::json::object();

	if (!legacy.empty())
	{
		j["fp_filenames"] = legacy.toStrArray();
	}
}

bool kson::LegacyAudioInfo::empty() const
{
	return bgm.empty();
}

void kson::to_json(nlohmann::json& j, const LegacyAudioInfo& legacy)
{
	j = nlohmann::json::object();

	if (!legacy.bgm.empty())
	{
		j["bgm"] = legacy.bgm;
	}
}
