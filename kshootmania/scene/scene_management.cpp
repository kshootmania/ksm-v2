#include "scene_management.hpp"
#include "title/title_scene.hpp"
#include "option/option_scene.hpp"

namespace SceneManagement
{
	SceneManager<StringView> MakeSceneManager()
	{
		SceneManager<StringView> sceneManager;

		sceneManager.add<TitleScene>(SceneName::kTitle);
		sceneManager.add<OptionScene>(SceneName::kOption);

		sceneManager.changeScene(SceneName::kTitle, kDefaultTransitionMs);

		return sceneManager;
	}
}
