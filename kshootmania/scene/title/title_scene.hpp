#pragma once
#include "scene.hpp"
#include "title_menu.hpp"

class TitleScene : public SceneManager<StringView>::Scene
{
private:
	Texture m_bgTexture;
	TitleMenu m_menu;

public:
	static constexpr StringView kSceneName = U"Title";

	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
