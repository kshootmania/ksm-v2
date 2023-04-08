#include "scene_manager_addon.hpp"
#include "title/title_scene.hpp"
#include "option/option_scene.hpp"
#include "select/select_scene.hpp"
#include "play/play_scene.hpp"

SceneManagerAddon::SceneManagerAddon()
{
	m_sceneManager.add<TitleScene>(SceneName::kTitle);
	m_sceneManager.add<OptionScene>(SceneName::kOption);
	m_sceneManager.add<SelectScene>(SceneName::kSelect);
	m_sceneManager.add<PlayScene>(SceneName::kPlay);
	m_sceneManager.changeScene(SceneName::kTitle, kDefaultTransitionMs);
}

bool SceneManagerAddon::update()
{
	return m_sceneManager.updateScene();
}

inline void SceneManagerAddon::draw() const
{
	m_sceneManager.drawScene();
}
