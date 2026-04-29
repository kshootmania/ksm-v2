#include "IMEUtils.hpp"

#ifdef _WIN32
#include <Windows.h>

namespace
{
	constexpr WPARAM kIMCSetOpenStatus = 0x0006;

	void SetIMEOpenStatus(HWND hWnd, BOOL open)
	{
		if (hWnd == nullptr)
		{
			return;
		}

		if (const HIMC imeContext = ImmGetContext(hWnd))
		{
			ImmSetOpenStatus(imeContext, open);

			if (!open)
			{
				ImmNotifyIME(imeContext, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
				ImmNotifyIME(imeContext, NI_CLOSECANDIDATE, 0, 0);
			}

			ImmReleaseContext(hWnd, imeContext);
		}

		if (const HWND imeHWnd = ImmGetDefaultIMEWnd(hWnd))
		{
			SendMessage(imeHWnd, WM_IME_CONTROL, kIMCSetOpenStatus, open);
		}
	}
}

void IMEUtils::DetachIMEContext()
{
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	if (hWnd == nullptr)
	{
		return;
	}

	SetIMEOpenStatus(hWnd, FALSE);
}

void IMEUtils::AttachIMEContext()
{
	const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
	if (hWnd == nullptr)
	{
		return;
	}

	SetIMEOpenStatus(hWnd, TRUE);
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
