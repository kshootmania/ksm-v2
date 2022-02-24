#include "scene.hpp"

namespace SceneManagement
{
	SceneManager<StringView> MakeSceneManager()
	{
		SceneManager<StringView> sceneManager;

		sceneManager.add<TitleScene>(TitleScene::kSceneName);
		sceneManager.add<OptionScene>(OptionScene::kSceneName);

		sceneManager.changeScene(TitleScene::kSceneName, kDefaultTransitionMs);

		return sceneManager;
	}
}
