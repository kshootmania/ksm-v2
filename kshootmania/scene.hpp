#pragma once
#include <Siv3D.hpp>
#include "scene/title/title_scene.hpp"

namespace SceneManagement
{
	SceneManager<String> MakeSceneManager();

	template <class SceneType>
	bool ChangeScene(SceneManager<String>& sceneManager)
	{
		return sceneManager.changeScene(SceneType::kSceneName);
	}
};
