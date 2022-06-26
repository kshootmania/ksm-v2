#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct DifficultyInfo
	{
		std::int32_t idx = 0; // 0-3 in KSH
	};

	struct MetaInfo
	{
		std::string title;
		std::string titleImgFilename; // UTF-8 guaranteed
		std::string artist;
		std::string artistImgFilename; // UTF-8 guaranteed
		std::string chartAuthor;
		DifficultyInfo difficulty;
		std::int32_t level = 1;
		std::string dispBPM;
		double stdBPM = 0.0;
		std::string jacketFilename; // UTF-8 guaranteed
		std::string jacketAuthor;
		std::string iconFilename; // UTF-8 guaranteed
		std::string information;
	};
}
