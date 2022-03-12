#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KSHUnknownOptionLine
	{
		std::u8string key;
		std::u8string value;
	};

	struct ImplRoot
	{
		ByPulseMulti<KSHUnknownOptionLine> kshUnknownOptionLines;

		// Note: "//" or ";" is not removed from the string.
		ByPulseMulti<std::u8string> kshComments;
	};
}
