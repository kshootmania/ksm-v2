#include "MathUtils.hpp"

[[nodiscard]]
int32 MathUtils::NumDigits(int32 number) noexcept
{
	return number == 0 ? 1 : static_cast<int32>(Log10(Abs(number))) + 1;
}
