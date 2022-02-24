#pragma once
#include <Siv3D.hpp>
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"

namespace SceneManagement
{
	constexpr int32 kDefaultTransitionMs = 800;

	SceneManager<StringView> MakeSceneManager();

	template <class SceneType>
	bool ChangeScene(SceneManager<StringView>& sceneManager)
	{
		return sceneManager.changeScene(SceneType::kSceneName);
	}
};
