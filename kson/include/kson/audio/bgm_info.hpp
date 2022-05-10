#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct BGMInfo
	{
		std::string filename; // UTF-8 guaranteed

		double vol = 1.0;

		int64_t offsetMs = 0;

		std::string previewFilename;

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};

	void to_json(nlohmann::json& j, const BGMInfo& bgm);

	struct MetaBGMInfo
	{
		double vol = 1.0;

		std::string previewFilename; // UTF-8 guaranteed

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};
}
