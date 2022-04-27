#include "math_utils.hpp"

ksh::Ms MathUtils::SecToMs(double sec)
{
	return sec * 1000;
}

double MathUtils::MsToSec(ksh::Ms sec)
{
	return sec / 1000;
}

int32 MathUtils::NumDigits(int32 number)
{
	return static_cast<int32>(Log10(number) + 1.0);
}
