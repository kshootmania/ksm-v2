#include "kson/audio/bgm_info.hpp"

bool kson::LegacyBGMInfo::empty() const
{
	return filenameF.empty() && filenameP.empty() && filenameFP.empty();
}

std::vector<std::string> kson::LegacyBGMInfo::toStrArray() const
{
	if (!filenameFP.empty())
	{
		return {
			filenameF,
			filenameP,
			filenameFP,
		};
	}
	else if (!filenameP.empty())
	{
		return {
			filenameF,
			filenameP,
		};
	}
	else if (!filenameF.empty())
	{
		return {
			filenameF,
		};
	}
	else
	{
		return std::vector<std::string>();
	}
}
