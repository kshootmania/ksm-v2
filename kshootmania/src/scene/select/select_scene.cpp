#include "select_scene.hpp"

void SelectScene::moveToPlayScene(FilePathView chartFilePath)
{
	getData().playSceneArgs.chartFilePath = chartFilePath;
	ScreenFadeAddon::FadeOutToScene(SceneName::kPlay, ScreenFadeAddon::kDefaultDuration, Palette::White);
}

SelectScene::SelectScene(const InitData& initData)
	: MyScene(initData)
	, m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? KeyConfig::kBack
			: KeyConfig::kBackspace)
	, m_menu([this](FilePathView chartFilePath) { moveToPlayScene(chartFilePath); })
{
	ScreenFadeAddon::FadeIn();
}

void SelectScene::update()
{
	if (ScreenFadeAddon::IsFadingOut())
	{
		return;
	}

	const bool closeFolder = m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* ← kBackspace・kBackのいずれかが入っている */);

	// Backボタン(Escキー)を押した場合、(フォルダを閉じる状況でなければ)タイトル画面へ戻る
	if (!closeFolder && KeyConfig::Down(KeyConfig::kBack))
	{
		ScreenFadeAddon::FadeOutToScene(SceneName::kTitle);
		return;
	}

	m_menu.update();

	// BackSpaceキーまたはBackボタン(Escキー)でフォルダを閉じる
	if (closeFolder)
	{
		m_menu.closeFolder(PlaySeYN::No);
	}

	// スタートボタンを押した場合、フォルダを開く または プレイ開始
	if (KeyConfig::Down(KeyConfig::kStart))
	{
		m_menu.decide();
	}
}

void SelectScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_bgAnim.draw();
	m_menu.draw();
}
