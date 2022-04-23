#pragma once
#include "ksh/common/common.hpp"

namespace ksh
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
