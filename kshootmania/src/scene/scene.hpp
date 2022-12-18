#pragma once
#include "play/play_scene_args.hpp"

struct MySceneData
{
	PlaySceneArgs playSceneArgs;
};

using MySceneManager = SceneManager<StringView, MySceneData>;
using MyScene = SceneManager<StringView, MySceneData>::Scene;

namespace SceneName
{
	constexpr StringView kTitle = U"Title";
	constexpr StringView kOption = U"Option";
	constexpr StringView kSelect = U"Select";
	constexpr StringView kPlay = U"Play";
}

constexpr int32 kDefaultTransitionMs = 800;
