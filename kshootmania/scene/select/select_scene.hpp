#pragma once
#include "select_bg_anim.hpp"

class SelectScene : public SceneManager<StringView>::Scene
{
private:
	const Texture m_bgTexture;
	const SelectBGAnim m_bgAnim;

public:
	explicit SelectScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
