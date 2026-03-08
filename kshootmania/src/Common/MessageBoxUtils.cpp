#include "MessageBoxUtils.hpp"
#ifdef _WIN32
#include <Windows.h>
#else
#include <ksmplatform_macos/input_method.h>
#endif

#ifdef _WIN32

namespace
{
	/// @brief MessageBoxStyleからWindowsのアイコンフラグに変換
	UINT ToIconFlag(const MessageBoxStyle style)
	{
		switch (style)
		{
		case MessageBoxStyle::Info:
			return MB_ICONINFORMATION;
		case MessageBoxStyle::Warning:
			return MB_ICONWARNING;
		case MessageBoxStyle::Error:
			return MB_ICONERROR;
		case MessageBoxStyle::Question:
			return MB_ICONQUESTION;
		default:
			return 0;
		}
	}
}
#endif

MessageBoxResult MessageBoxUtils::ShowOK(const StringView text, const MessageBoxStyle style)
{
#ifdef _WIN32
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	::MessageBoxW(hWnd, text.toWstr().c_str(), Window::GetTitle().toWstr().c_str(), MB_OK | ToIconFlag(style));
	::SetForegroundWindow(hWnd);
	return MessageBoxResult::OK;
#else
	const auto result = System::MessageBoxOK(text, style);
	KSMPlatformMacOS_ActivateWindow();
	return result;
#endif
}

MessageBoxResult MessageBoxUtils::ShowYesNo(const StringView text, const MessageBoxStyle style)
{
#ifdef _WIN32
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	const int result = ::MessageBoxW(hWnd, text.toWstr().c_str(), Window::GetTitle().toWstr().c_str(), MB_YESNO | ToIconFlag(style));
	::SetForegroundWindow(hWnd);
	return result == IDYES ? MessageBoxResult::OK : MessageBoxResult::Cancel;
#else
	const auto result = System::MessageBoxOKCancel(text, style);
	KSMPlatformMacOS_ActivateWindow();
	return result;
#endif
}
