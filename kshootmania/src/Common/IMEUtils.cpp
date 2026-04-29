#include "IMEUtils.hpp"

#ifdef _WIN32
#include <Windows.h>

void IMEUtils::DetachIMEContext()
{
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	if (hWnd != NULL)
	{
		ImmAssociateContextEx(hWnd, NULL, 0);
	}
}

void IMEUtils::AttachIMEContext()
{
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	if (hWnd != NULL)
	{
		ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
	}
}
#elif defined(__APPLE__)
#include <ksmplatform_macos/input_method.h>

void IMEUtils::DetachIMEContext()
{
	KSMPlatformMacOS_DisableIME();
}

void IMEUtils::AttachIMEContext()
{
	// macOSは入力ソース切替方式のため、再アタッチ処理は不要
	// (編集中はDisableIMEAddon側でスキップしているのでユーザーが自由にIMEを切替可能)
}
#endif
