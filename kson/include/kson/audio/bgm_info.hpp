#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct BGMPreviewInfo
	{
		std::int32_t offset = 0;

		std::int32_t duration = 15000;
	};

	struct LegacyBGMInfo
	{
		// Filenames of pre-rendered BGM with audio effects from legacy KSH charts
		std::string filenameF; // "xxx_f.ogg", UTF-8 guaranteed
		std::string filenameP; // "xxx_p.ogg", UTF-8 guaranteed
		std::string filenameFP; // "xxx_fp.ogg", UTF-8 guaranteed

		bool empty() const;

		std::vector<std::string> toStrArray() const;
	};

	struct BGMInfo
	{
		std::string filename; // UTF-8 guaranteed

		double vol = 1.0;

		std::int32_t offset = 0;

		BGMPreviewInfo preview;

		LegacyBGMInfo legacy;
	};

	struct MetaBGMInfo
	{
		double vol = 1.0;

		BGMPreviewInfo preview;
	};
}
