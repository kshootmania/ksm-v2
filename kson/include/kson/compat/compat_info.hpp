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

	struct CompatInfo
	{
		std::string kshVersion;
		KSHUnknownInfo kshUnknown;

		bool empty() const;
	};

	struct MetaCompatInfo
	{
		std::string kshVersion;
	};
}
