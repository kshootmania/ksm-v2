#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene.hpp"
#include "scene/title/title_assets.hpp"

void Main()
{
	TitleAssets::RegisterAssets();

	SceneManager<StringView> sceneManager = SceneManagement::MakeSceneManager();

	while (System::Update())
	{
		if (!sceneManager.update())
		{
			break;
		}
	}
}
