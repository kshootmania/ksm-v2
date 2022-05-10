#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHUnknownInfo
	{
		std::unordered_map<std::string, std::string> meta;
		std::unordered_map<std::string, ByPulseMulti<std::string>> option;
		ByPulseMulti<std::string> line;

		bool empty() const;
	};

	struct MetaCompatInfo
	{
		std::string kshVersion;
	};

	struct CompatInfo
	{
		std::string kshVersion;
		KSHUnknownInfo kshUnknown;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KSHUnknownInfo& unknown);
	void to_json(nlohmann::json& j, const CompatInfo& compat);
}
