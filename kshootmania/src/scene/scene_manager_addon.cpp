#include "scene_manager_addon.hpp"
#include "title/title_scene.hpp"
#include "option/option_scene.hpp"
#include "select/select_scene.hpp"
#include "play/play_scene.hpp"
#include "result/result_scene.hpp"

namespace
{
	SceneManagerAddon& GetInstance()
	{
		SceneManagerAddon* const pAddon = Addon::GetAddon<SceneManagerAddon>(U"SceneManager");
		if (pAddon == nullptr)
		{
			throw Error(U"SceneManager addon is not registered");
		}
		return *pAddon;
	}
}

bool SceneManagerAddon::update()
{
	return m_sceneManager.updateScene();
}

void SceneManagerAddon::draw() const
{
	m_sceneManager.drawScene();
}

void SceneManagerAddon::Init()
{
	MySceneManager& sceneManager = GetInstance().m_sceneManager;
	sceneManager.add<TitleScene>(SceneName::kTitle);
	sceneManager.add<OptionScene>(SceneName::kOption);
	sceneManager.add<SelectScene>(SceneName::kSelect);
	sceneManager.add<PlayScene>(SceneName::kPlay);
	sceneManager.add<ResultScene>(SceneName::kResult);
	ScreenFadeAddon::FadeOut(0s);
	sceneManager.init(SceneName::kTitle, 0);
}

void SceneManagerAddon::ChangeScene(const String& sceneName)
{
	GetInstance().m_sceneManager.changeScene(sceneName, 0);
}
