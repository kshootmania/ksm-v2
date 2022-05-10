#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct EditorInfo
	{
		ByPulse<std::string> comment;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const EditorInfo& legacy);
}
