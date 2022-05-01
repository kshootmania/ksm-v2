#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct DifficultyInfo
	{
		std::int8_t idx = 0; // 0-3 in KSH
	};

	void to_json(nlohmann::json& j, const DifficultyInfo& difficulty);

	struct LegacyMetaInfo
	{
		std::string titleImageFilename; // UTF-8 guaranteed
		std::string artistImageFilename; // UTF-8 guaranteed

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const LegacyMetaInfo& legacy);

	struct MetaRoot
	{
		std::string title;
		std::string titleTranslit;
		std::string subtitle;
		std::string artist;
		std::string artistTranslit;
		std::string chartAuthor;
		DifficultyInfo difficulty;
		std::int8_t level = 1;
		std::string dispBPM;
		double standardBPM = 0.0;
		std::string jacketFilename; // UTF-8 guaranteed
		std::string jacketAuthor;
		std::string information;
		LegacyMetaInfo legacy;
		std::string kshVersion;
	};

	void to_json(nlohmann::json& j, const MetaRoot& meta);
}
