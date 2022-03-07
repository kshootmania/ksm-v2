#pragma once
#include <Siv3D.hpp>

namespace SceneName
{
	constexpr StringView kTitle = U"Title";
	constexpr StringView kOption = U"Option";
}

namespace SceneManagement
{
	constexpr int32 kDefaultTransitionMs = 800;

	SceneManager<StringView> MakeSceneManager();
};
