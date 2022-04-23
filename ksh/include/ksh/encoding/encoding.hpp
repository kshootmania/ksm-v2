#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	namespace Encoding
	{
		std::string ShiftJISToUTF8(std::string_view shiftJISStr);
	}
}
