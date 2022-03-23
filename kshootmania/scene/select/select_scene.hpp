#pragma once
#include "select_bg_anim.hpp"
#include "select_menu.hpp"
#include "select_folder_state.hpp"

class SelectScene : public SceneManager<StringView>::Scene
{
private:
	const Texture m_bgTexture;
	const SelectBGAnim m_bgAnim;
	const KeyConfig::Button m_folderCloseButton;

	SelectMenu m_menu;

public:
	explicit SelectScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
