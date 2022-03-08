#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct BGMInfo
	{
		String filename;

		double volume = 1.0;

		int64_t offsetMs = 0;

		String previewFilename;

		int64_t previewOffsetMs = 0;

		int64_t previewDurationMs = 0;
	};
}
