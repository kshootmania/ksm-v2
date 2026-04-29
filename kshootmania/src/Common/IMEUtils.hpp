#pragma once

#if defined(_WIN32) || defined(__APPLE__)
namespace IMEUtils
{
	void DetachIMEContext();

	void AttachIMEContext();
}
#endif
