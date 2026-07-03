#include "MouseMenuUtils.hpp"

namespace MouseMenuUtils
{
	Optional<int32> FiredClickIndex(const noco::Canvas& canvas, const Array<std::pair<String, int32>>& tagToCursorPairs)
	{
		for (const auto& [tag, cursorValue] : tagToCursorPairs)
		{
			if (canvas.isEventFiredWithTag(tag))
			{
				return cursorValue;
			}
		}
		return none;
	}
}
