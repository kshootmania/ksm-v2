#include "kson/camera/tilt.hpp"

bool kson::TiltRoot::empty() const
{
	return scale.empty() && manual.empty() && keep.empty();
}

void kson::to_json(nlohmann::json& j, const TiltRoot& tilt)
{
	j = nlohmann::json::object();

	if (!tilt.scale.empty())
	{
		j["scale"] = tilt.scale;
	}

	if (!tilt.manual.empty())
	{
		j["manual"] = tilt.manual;
	}

	if (!tilt.keep.empty())
	{
		auto keep = nlohmann::json::array();

		// Skip the same value
		// (The first "false" is also skipped)
		bool prevValue = false;
		for (const auto& [y, v] : tilt.keep)
		{
			if (v == prevValue)
			{
				continue;
			}

			keep.push_back({
				{ "y", y },
				{ "v", v },
			});

			prevValue = v;
		}

		j["keep"] = keep;
	}
}
