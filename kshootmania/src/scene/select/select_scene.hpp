#pragma once
#include "select_assets.hpp"
#include "select_bg_anim.hpp"
#include "select_menu.hpp"
#include "select_folder_state.hpp"

class SelectScene : public MyScene
{
private:
	const Texture m_bgTexture{ TextureAsset(SelectTexture::kBG) };
	const SelectBGAnim m_bgAnim;
	const KeyConfig::Button m_folderCloseButton;

	SelectMenu m_menu;

	void moveToPlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay);

public:
	explicit SelectScene(const InitData& initData);

	virtual void update() override;

	virtual void updateFadeIn([[maybe_unused]] double t) { update(); }

	virtual void draw() const override;
};
