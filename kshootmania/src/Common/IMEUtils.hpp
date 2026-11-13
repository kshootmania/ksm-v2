#pragma once

#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)
namespace IMEUtils
{
	void DetachIMEContext();

	void AttachIMEContext();
}
#endif
