#pragma once
#include "play/play_scene_args.hpp"

struct MySceneData
{
	PlaySceneArgs playSceneArgs;
};

using MySceneManager = SceneManager<String, MySceneData>;
using MyScene = MySceneManager::Scene;

namespace SceneName
{
	inline const String kTitle = U"Title";
	inline const String kOption = U"Option";
	inline const String kSelect = U"Select";
	inline const String kPlay = U"Play";
}

inline constexpr int32 kDefaultTransitionMs = 800;

class SceneManagerAddon : public IAddon
{
private:
	MySceneManager m_sceneManager;

public:
	SceneManagerAddon();

	virtual bool update() override;

	virtual void draw() const override;
};
