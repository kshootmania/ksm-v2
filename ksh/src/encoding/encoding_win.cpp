#ifdef _WIN32
#include "ksh/encoding/encoding.hpp"
#include <Windows.h>

namespace
{
	// Shift-JIS (Japanese) codepage
	// Note: CP_ACP is not used for non-Japanese locales.
	//       Almost all non-UTF8 legacy charts are in Japanese, so they are always considered Shift-JIS here.
	constexpr UINT kShiftJISCodePage = 932;
}

std::u8string ksh::Encoding::ShiftJISToUTF8(std::string_view shiftJISStr)
{
	// Convert Shift-JIS to UTF-16
	const int requiredWstrSize = MultiByteToWideChar(932, 0, shiftJISStr.data(), static_cast<int>(shiftJISStr.size()), nullptr, 0);
	std::wstring wstr(requiredWstrSize, L'\0');
	MultiByteToWideChar(932, 0, shiftJISStr.data(), static_cast<int>(shiftJISStr.size()), wstr.data(), requiredWstrSize);

	// Convert UTF-16 to UTF-8
	const int requiredStrSize = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
	std::u8string str(requiredStrSize, u8'\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, reinterpret_cast<char *>(str.data()), requiredStrSize, nullptr, nullptr);

	// Remove an extra null terminator
	if (str.back() == u8'\0')
	{
		str.pop_back();
	}

	return str;
}

#endif
