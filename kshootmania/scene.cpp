#include "scene.hpp"

namespace
{
	constexpr int kDefaultTransitionMs = 500;
}

namespace SceneManagement
{
	SceneManager<StringView> MakeSceneManager()
	{
		SceneManager<StringView> sceneManager;

		sceneManager.add<TitleScene>(TitleScene::kSceneName);

		sceneManager.changeScene(TitleScene::kSceneName, kDefaultTransitionMs);

		return sceneManager;
	}
}
