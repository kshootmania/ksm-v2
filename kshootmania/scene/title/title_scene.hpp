#pragma once
#include "scene.hpp"
#include "title_menu.hpp"

class TitleScene : public SceneManager<String>::Scene
{
private:
	Texture m_bgTexture;
	TitleMenu m_menu;

public:
	static const String kSceneName;

	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
