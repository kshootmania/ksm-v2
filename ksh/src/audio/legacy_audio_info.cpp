#include "ksh/audio/legacy_audio_info.hpp"

std::vector<std::string> ksh::LegacyAudioBGMInfo::toStrArray() const
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

void ksh::to_json(nlohmann::json& j, const LegacyAudioBGMInfo& legacy)
{
	j = nlohmann::json::object();

	if (!legacy.empty())
	{
		j["fp_filenames"] = legacy.toStrArray();
	}
}

void ksh::to_json(nlohmann::json& j, const LegacyAudioInfo& legacy)
{
	j = nlohmann::json::object();

	if (!legacy.bgmInfo.empty())
	{
		j["bgm"] = legacy.bgmInfo;
	}
}
