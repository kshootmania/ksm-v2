#include "ksmplatform_linux/keyboard.h"
#include <X11/Xlib.h>
#include <cstring>

namespace
{
	Display* s_display = nullptr;
	bool s_isDisplayOpened = false;
	char s_keys[32] = {};
}

void KSMPlatformLinux_UpdateKeyboardState()
{
	if (!s_isDisplayOpened)
	{
		s_display = XOpenDisplay(nullptr);
		s_isDisplayOpened = true;
	}

	if (s_display == nullptr)
	{
		return;
	}

	if (XQueryKeymap(s_display, s_keys) == 0)
	{
		std::memset(s_keys, 0, sizeof(s_keys));
	}
}

bool KSMPlatformLinux_IsKeyPressed(unsigned long keysym)
{
	if (s_display == nullptr)
	{
		return false;
	}

	const KeyCode keyCode = XKeysymToKeycode(s_display, keysym);
	if (keyCode == 0)
	{
		return false;
	}

	return (s_keys[keyCode / 8] & (1 << (keyCode % 8))) != 0;
}
