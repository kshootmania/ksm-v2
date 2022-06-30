#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "ksmaudio/ksmaudio.hpp"

void Main()
{
	// Disable application termination by Esc key
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// Set default colors
	Scene::SetBackground(Palette::Black);
	Graphics3D::SetGlobalAmbientColor(Palette::White);
	Graphics3D::SetSunColor(Palette::Black);

	// Initialize audio backend
#ifdef _WIN32
	ksmaudio::Init(s3d::Platform::Windows::Window::GetHWND());
#else
	ksmaudio::Init(nullptr);
#endif

	// Load language text file
	I18n::LoadLanguage(U"Japanese");

	// Load config.ini
	ConfigIni::Load();

	// Register asset list
	AssetManagement::RegisterAssets();

	//Graphics::SetVSyncEnabled(false);

#if defined(_WIN32) && defined(_DEBUG)
	AllocConsole();
	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

	{
		// Create scene manager
		MySceneManager sceneManager;
		sceneManager.add<TitleScene>(SceneName::kTitle);
		sceneManager.add<OptionScene>(SceneName::kOption);
		sceneManager.add<SelectScene>(SceneName::kSelect);
		sceneManager.add<PlayScene>(SceneName::kPlay);
		sceneManager.changeScene(SceneName::kTitle, kDefaultTransitionMs);

		// Main loop
		while (System::Update())
		{
			if (!sceneManager.update())
			{
				break;
			}
		}
	}

	// Save config.ini
	ConfigIni::Save();

	// Terminate audio backend
	ksmaudio::Terminate();
}
