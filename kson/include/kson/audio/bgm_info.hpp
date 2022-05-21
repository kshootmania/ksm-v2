#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct BGMPreviewInfo
	{
		std::string filename; // UTF-8 guaranteed

		std::int32_t offset = 0;

		std::int32_t duration = 15000;
	};

	struct BGMInfo
	{
		std::string filename; // UTF-8 guaranteed

		double vol = 1.0;

		std::int32_t offset = 0;

		BGMPreviewInfo preview;
	};

	void to_json(nlohmann::json& j, const BGMInfo& bgm);

	struct MetaBGMInfo
	{
		double vol = 1.0;

		BGMPreviewInfo preview;
	};
}
