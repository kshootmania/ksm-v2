#include "kson/audio/bgm_info.hpp"

void kson::to_json(nlohmann::json& j, const BGMInfo& bgm)
{
	j = {
		{ "filename", bgm.filename },
		{ "volume", bgm.volume },
		{ "offset", bgm.offsetMs },
		{ "preview_offset", bgm.previewOffsetMs },
		{ "preview_duration", bgm.previewDurationMs },
	};

	if (bgm.filename != bgm.previewFilename)
	{
		j["preview_filename"] = bgm.previewFilename;
	}
}
