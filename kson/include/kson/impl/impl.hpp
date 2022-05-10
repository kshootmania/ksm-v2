#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHUnknownOptionLine
	{
		std::string key;
		std::string value;
	};

	struct ImplRoot
	{
		ByPulseMulti<KSHUnknownOptionLine> kshUnknownOptionLines;

		// Note: "//" or ";" is not removed from the string.
		ByPulseMulti<std::string> kshComments;
	};
}
