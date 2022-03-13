#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct DifficultyInfo
	{
		std::int8_t idx = 0; // 0-3 in KSH
	};

	struct LegacyMetaInfo
	{
		std::u8string titleImageFilename;
		std::u8string artistImageFilename;
	};

	struct MetaRoot
	{
		std::u8string title;
		std::u8string titleTranslit;
		std::u8string subtitle;
		std::u8string artist;
		std::u8string artistTranslit;
		std::u8string chartAuthor;
		DifficultyInfo difficulty;
		std::int8_t level = 1;
		std::u8string dispBPM;
		double standardBPM = 0.0;
		std::u8string jacketFilename;
		std::u8string jacketAuthor;
		std::u8string information;
		LegacyMetaInfo legacy;
		std::u8string kshVersion;
	};
}
