#include "math_utils.hpp"

int32 MathUtils::NumDigits(int32 number)
{
	if (number == 0)
	{
		return 1;
	}
	return static_cast<int32>(Log10(Abs(number))) + 1;
}
