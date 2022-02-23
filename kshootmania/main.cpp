#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene.hpp"
#include "scene/title/title_assets.hpp"

void Main()
{
	// Disable application termination by Esc key
	System::SetTerminationTriggers(UserAction::NoAction);

	// Register asset list
	// (Note: Assets are not loaded here. They are loaded at the time of use.)
	TitleAssets::RegisterAssets();

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
