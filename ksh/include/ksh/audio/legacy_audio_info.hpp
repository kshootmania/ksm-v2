#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LegacyAudioBGMInfo
	{
		// Filenames of pre-rendered BGM with audio effects from legacy KSH charts
		std::u8string filenameF; // "xxx_f.ogg"
		std::u8string filenameP; // "xxx_p.ogg"
		std::u8string filenameFP; // "xxx_fp.ogg"

		bool empty() const
		{
			return filenameF.empty() && filenameP.empty() && filenameFP.empty();
		}

		std::vector<std::string> toStrArray() const
		{
			if (!filenameFP.empty())
			{
				return {
					UnU8(filenameF),
					UnU8(filenameP),
					UnU8(filenameFP),
				};
			}
			else if (!filenameP.empty())
			{
				return {
					UnU8(filenameF),
					UnU8(filenameP),
				};
			}
			else if (!filenameF.empty())
			{
				return {
					UnU8(filenameF),
				};
			}
			else
			{
				return std::vector<std::string>();
			}
		}
	};

	inline void to_json(nlohmann::json& j, const LegacyAudioBGMInfo& legacy)
	{
		j = nlohmann::json::object();
		
		if (!legacy.empty())
		{
			j["fp_filenames"] = legacy.toStrArray();
		}
	}

	struct LegacyAudioInfo
	{
		LegacyAudioBGMInfo bgmInfo;

		bool empty() const
		{
			return bgmInfo.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const LegacyAudioInfo& legacy)
	{
		j = nlohmann::json::object();

		if (!legacy.bgmInfo.empty())
		{
			j["bgm"] = legacy.bgmInfo;
		}
	}
}
