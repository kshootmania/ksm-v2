#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LegacyAudioBGMInfo
	{
		// Filenames of pre-rendered BGM with audio effects from legacy KSH charts
		std::string filenameF; // "xxx_f.ogg", UTF-8 guaranteed
		std::string filenameP; // "xxx_p.ogg", UTF-8 guaranteed
		std::string filenameFP; // "xxx_fp.ogg", UTF-8 guaranteed

		bool empty() const
		{
			return filenameF.empty() && filenameP.empty() && filenameFP.empty();
		}

		std::vector<std::string> toStrArray() const
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
