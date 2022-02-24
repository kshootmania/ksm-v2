#pragma once
#include "scene.hpp"
#include "option_menu.hpp"

class OptionScene : public SceneManager<StringView>::Scene
{
private:
	Texture m_bgTexture;
	OptionMenu m_menu;
	Font m_font;

public:
	static constexpr StringView kSceneName = U"Option";

	explicit OptionScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	void exitScene();
};
