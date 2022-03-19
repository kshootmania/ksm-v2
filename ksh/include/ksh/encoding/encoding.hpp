#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	namespace Encoding
	{
		std::u8string ShiftJISToUTF8(std::string_view shiftJISStr);
	}

	// HACK: The functions to reinterpret u8string as string for use of nlohmann/json.
	inline std::string_view UnU8(std::u8string_view u8str)
	{
		return std::string_view(reinterpret_cast<const char*>(u8str.data()), u8str.size());
	}
	inline const std::string& UnU8(const std::u8string& u8str)
	{
		return *reinterpret_cast<const std::string*>(&u8str);
	}
	inline const char* UnU8(const char8_t* u8str)
	{
		return reinterpret_cast<const char*>(u8str);
	}
	template <typename T>
	const std::unordered_map<std::string, T>& UnU8(const std::unordered_map<std::u8string, T>& u8map)
	{
		return *reinterpret_cast<const std::unordered_map<std::string, T>*>(&u8map);
	}
}
