#include "ScreenUtils.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

TextureRegion ScreenUtils::FitToHeight(const Texture& texture)
{
	const double height = Scene::Height();
	const double width = texture.width() * height / texture.height();
	return texture.resized(width, height);
}

void ScreenUtils::ApplyScreenSizeConfig()
{
	const bool isFullScreen = ConfigIni::GetBool(ConfigIni::Key::kFullScreen, false);
	const String resolutionStr{ ConfigIni::GetString(isFullScreen ? ConfigIni::Key::kFullScreenResolution : ConfigIni::Key::kWindowResolution, U"800,600") };
	const Array<int32> resolution = resolutionStr.split(U',').map([](const String& str) { return ParseOr<int32>(str, 0); });
	Scene::SetResizeMode(ResizeMode::Keep);
	
	if (resolution.size() == 2U && resolution[0] > 0 && resolution[1] > 0)
	{
		Scene::Resize(resolution[0], resolution[1]);
		Window::Resize(resolution[0], resolution[1]);
	}
	else
	{
		Scene::Resize(800, 600);
		Window::Resize(800, 600);
	}
	Window::SetFullscreen(isFullScreen);

#ifdef _WIN32
	// 他のウィンドウに切り替えられるようTOPMOSTを解除
	if (isFullScreen)
	{
		const auto hWnd = static_cast<HWND>(Platform::Windows::Window::GetHWND());
		if (hWnd != NULL)
		{
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
#endif
}
