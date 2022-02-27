#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene.hpp"
#include "scene/title/title_assets.hpp"
#include "scene/option/option_assets.hpp"
#include "i18n.hpp"
#include "config_ini.hpp"

void Main()
{
	// Disable application termination by Esc key
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// Register asset list
	// (Note: Assets are not loaded here. They are loaded at the time of use.)
	TitleAssets::RegisterAssets();
	OptionAssets::RegisterAssets();

	// Load language text file
	I18n::LoadLanguage(U"Japanese");

	// Load config.ini
	ConfigIni::Load();

	// Main loop
	SceneManager<StringView> sceneManager = SceneManagement::MakeSceneManager();
	while (System::Update())
	{
		if (!sceneManager.update())
		{
			break;
		}
	}
}
