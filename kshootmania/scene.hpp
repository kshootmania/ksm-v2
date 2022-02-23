﻿#pragma once
#include <Siv3D.hpp>
#include "scene/title/title_scene.hpp"

namespace SceneManagement
{
	SceneManager<StringView> MakeSceneManager();

	template <class SceneType>
	bool ChangeScene(SceneManager<StringView>& sceneManager)
	{
		return sceneManager.changeScene(SceneType::kSceneName);
	}
};