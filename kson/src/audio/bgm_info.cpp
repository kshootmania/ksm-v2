#include "kson/audio/bgm_info.hpp"

void kson::to_json(nlohmann::json& j, const BGMInfo& bgm)
{
	j = {
		{ "filename", bgm.filename },
		{ "vol", bgm.vol },
		{ "offset", bgm.offset },
		{ "preview", {
			{ "offset", bgm.preview.offset },
			{ "duration", bgm.preview.duration },
		}},
	};

	if (bgm.preview.filename != bgm.filename)
	{
		j["preview"]["filename"] = bgm.preview.filename;
	}
}
