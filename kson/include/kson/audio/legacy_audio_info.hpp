#pragma once
#include "kson/common/common.hpp"

namespace kson
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

		std::vector<std::string> toStrArray() const;
	};

	void to_json(nlohmann::json& j, const LegacyAudioBGMInfo& legacy);

	struct LegacyAudioInfo
	{
		LegacyAudioBGMInfo bgmInfo;

		bool empty() const
		{
			return bgmInfo.empty();
		}
	};

	void to_json(nlohmann::json& j, const LegacyAudioInfo& legacy);
}
