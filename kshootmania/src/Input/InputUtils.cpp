#include "InputUtils.hpp"

namespace
{
	int32 s_lastUpdatedWindowFocusFrameCount = -1;
	bool s_prevWindowFocused = true;
	bool s_windowFocusedThisFrame = false;
}

namespace InputUtils
{
	ksmaxis::DeviceFlags GetLaserInputDeviceFlags()
	{
		const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
		switch (laserInputType)
		{
		case ConfigIni::Value::LaserInputType::kMouseXY:
			return ksmaxis::GetRequiredDeviceFlags(ksmaxis::InputMode::kMouse);
		case ConfigIni::Value::LaserInputType::kSlider:
			return ksmaxis::GetRequiredDeviceFlags(ksmaxis::InputMode::kSlider);
		case ConfigIni::Value::LaserInputType::kAnalogStickXY:
			return ksmaxis::GetRequiredDeviceFlags(ksmaxis::InputMode::kAnalogStick);
		default:
			return ksmaxis::DeviceFlags::None;
		}
	}

	void InitKsmaxisForCurrentLaserInput()
	{
		const ksmaxis::DeviceFlags deviceFlags = GetLaserInputDeviceFlags();
		if (deviceFlags != ksmaxis::DeviceFlags::None)
		{
			std::string ksmaxisError;
			std::vector<std::string> ksmaxisWarnings;
#ifdef _WIN32
			const bool ksmaxisSuccess = ksmaxis::Init(deviceFlags, s3d::Platform::Windows::Window::GetHWND(), &ksmaxisError, &ksmaxisWarnings);
#else
			const bool ksmaxisSuccess = ksmaxis::Init(deviceFlags, &ksmaxisError, &ksmaxisWarnings);
#endif
			if (!ksmaxisSuccess)
			{
				Logger << U"[ksm error] ksmaxis::Init() failed: " << Unicode::FromUTF8(ksmaxisError);
			}
			for (const auto& warning : ksmaxisWarnings)
			{
				Logger << U"[ksm warning] ksmaxis: " << Unicode::FromUTF8(warning);
			}
		}
	}

	void UpdateWindowFocusState()
	{
		const int32 currentFrameCount = Scene::FrameCount();
		if (s_lastUpdatedWindowFocusFrameCount == currentFrameCount)
		{
			return;
		}
		s_lastUpdatedWindowFocusFrameCount = currentFrameCount;

		const bool windowFocused = Window::GetState().focused;
		s_windowFocusedThisFrame = windowFocused && !s_prevWindowFocused;
		s_prevWindowFocused = windowFocused;
	}

	bool WindowFocusedThisFrame()
	{
		// メインループより先にコルーチン側から参照された場合もあるため、参照時点でも更新する
		UpdateWindowFocusState();
		return s_windowFocusedThisFrame;
	}
}
