#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct BGMInfo
	{
		std::u8string filename;

		double volume = 1.0;

		int64_t offsetMs = 0;

		std::u8string previewFilename;

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};

	inline void to_json(nlohmann::json& j, const BGMInfo& bgm)
	{
		j = {
			{ "filename", UnU8(bgm.filename) },
			{ "volume", bgm.volume },
			{ "offset", bgm.offsetMs },
			{ "preview_offset", bgm.previewOffsetMs },
			{ "preview_duration", bgm.previewDurationMs },
		};

		if (bgm.filename != bgm.previewFilename)
		{
			j["preview_filename"] = UnU8(bgm.previewFilename);
		}
	}
	/*
	inline void from_json(const nlohmann::json& j, BGMInfo& bgm)
	{
		j.at("filename").get_to(bgm.filename);
		j.at("volume").get_to(bgm.volume);
		j.at("offset").get_to(bgm.offsetMs);
		j.at("preview_filename").get_to(bgm.previewFilename);
		j.at("preview_offset").get_to(bgm.previewOffsetMs);
		j.at("preview_duration").get_to(bgm.previewDurationMs);
	}*/

	struct MetaBGMInfo
	{
		double volume = 1.0;

		std::u8string previewFilename;

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};
}
