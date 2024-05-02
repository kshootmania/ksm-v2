#pragma once
#include <CoTaskLib.hpp>
#include "select_assets.hpp"
#include "select_bg_anim.hpp"
#include "select_menu.hpp"
#include "select_folder_state.hpp"

class SelectScene : public Co::UpdateSceneBase
{
private:
	const Texture m_bgTexture{ TextureAsset(SelectTexture::kBG) };
	const SelectBGAnim m_bgAnim;
	const KeyConfig::Button m_folderCloseButton;

	ColorF m_fadeOutColor = Palette::Black;

	SelectMenu m_menu;

	void moveToPlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay);

public:
	SelectScene();

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
