#pragma once
#include "play/play_scene_args.hpp"
#include "result/result_scene_args.hpp"

struct MySceneData
{
	// TODO: 若干デカめなのとライフサイクルが長いと良くないのでunique_ptrにした方がよさそう
	PlaySceneArgs playSceneArgs;
	ResultSceneArgs resultSceneArgs;
};

using MySceneManager = SceneManager<String, MySceneData>;
using MyScene = MySceneManager::Scene;

namespace SceneName
{
	inline const String kTitle = U"Title";
	inline const String kOption = U"Option";
	inline const String kSelect = U"Select";
	inline const String kPlay = U"Play";
	inline const String kResult = U"Result";
}

constexpr int32 kDefaultTransitionMs = 800;

class SceneManagerAddon : public IAddon
{
private:
	MySceneManager m_sceneManager;

public:
	SceneManagerAddon();

	virtual bool update() override;

	virtual void draw() const override;
};
