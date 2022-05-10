#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	namespace Encoding
	{
		std::string ShiftJISToUTF8(std::string_view shiftJISStr);
	}
}
