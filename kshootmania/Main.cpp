#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene.hpp"

void Main()
{
	SceneManager<String> sceneManager = SceneManagement::MakeSceneManager();

	while (System::Update())
	{
		if (!sceneManager.update())
		{
			break;
		}
	}
}
