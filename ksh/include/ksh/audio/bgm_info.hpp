#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct BGMInfo
	{
		std::string filename; // UTF-8 guaranteed

		double volume = 1.0;

		int64_t offsetMs = 0;

		std::string previewFilename;

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};

	void to_json(nlohmann::json& j, const BGMInfo& bgm);

	struct MetaBGMInfo
	{
		double volume = 1.0;

		std::string previewFilename; // UTF-8 guaranteed

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};
}
