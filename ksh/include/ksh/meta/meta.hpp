#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct DifficultyInfo
	{
		int8_t idx; // 0-3 in KSH
	};

	struct LegacyMetaInfo
	{
		String titleImageFilename;
		String artistImageFilename;
	};

	struct MetaRoot
	{
		String title;
		String titleTranslit;
		String subtitle;
		String artist;
		String artistTranslit;
		String chartAuthor;
		DifficultyInfo difficulty;
		int8_t level = 1;
		String dispBPM;
		double standardBPM = 0.0;
		String jacketFilename;
		String jacketAuthor;
		String information;
		LegacyMetaInfo legacy;
		String kshVersion;
	};
}
