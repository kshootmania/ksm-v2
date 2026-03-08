#include "MessageBoxUtils.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

MessageBoxResult MessageBoxUtils::ShowOK(const StringView text, const MessageBoxStyle style)
{
#ifdef _WIN32
	UINT iconFlag = 0;
	switch (style)
	{
	case MessageBoxStyle::Info:
		iconFlag = MB_ICONINFORMATION;
		break;
	case MessageBoxStyle::Warning:
		iconFlag = MB_ICONWARNING;
		break;
	case MessageBoxStyle::Error:
		iconFlag = MB_ICONERROR;
		break;
	case MessageBoxStyle::Question:
		iconFlag = MB_ICONQUESTION;
		break;
	default:
		break;
	}

	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	::MessageBoxW(hWnd, text.toWstr().c_str(), Window::GetTitle().toWstr().c_str(), MB_OK | iconFlag);

	return MessageBoxResult::OK;
#else
	return System::MessageBoxOK(text, style);
#endif
}
