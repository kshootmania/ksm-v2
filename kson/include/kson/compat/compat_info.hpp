#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHUnknownInfo
	{
		std::unordered_map<std::string, std::string> meta;
		std::unordered_map<std::string, ByPulseMulti<std::string>> option;
		ByPulseMulti<std::string> line;
	};

	struct CompatInfo
	{
		std::string kshVersion;
		KSHUnknownInfo kshUnknown;
	};

	struct MetaCompatInfo
	{
		std::string kshVersion;
	};
}
