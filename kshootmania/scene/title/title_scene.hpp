#pragma once
#include "scene.hpp"

class TitleScene : public SceneManager<String>::Scene
{
private:
	Texture m_bgTexture;

public:
	static const String kSceneName;

	explicit TitleScene(const InitData& initData);

	void update();

	void draw() const;
};
