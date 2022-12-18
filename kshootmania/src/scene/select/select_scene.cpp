#include "select_scene.hpp"
#include "select_assets.hpp"

void SelectScene::moveToPlayScene(FilePathView chartFilePath)
{
	getData().playSceneArgs.chartFilePath = chartFilePath;
	changeScene(SceneName::kPlay, kDefaultTransitionMs);
}

SelectScene::SelectScene(const InitData& initData)
	: MyScene(initData)
	, m_bgTexture(TextureAsset(SelectTexture::kBG))
	, m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? KeyConfig::kBack
			: KeyConfig::kBackspace)
	, m_menu([this](FilePathView chartFilePath) { moveToPlayScene(chartFilePath); })
{
}

void SelectScene::update()
{
	const bool closeFolder = m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* ← kBackspace・kBackのいずれかが入っている */);

	// Backボタン(Escキー)を押した場合、(フォルダを閉じる状況でなければ)タイトル画面へ戻る
	if (!closeFolder && KeyConfig::Down(KeyConfig::kBack))
	{
		changeScene(SceneName::kTitle, kDefaultTransitionMs);
		return;
	}

	m_menu.update();

	// BackSpaceキーまたはBackボタン(Escキー)でフォルダを閉じる
	if (closeFolder)
	{
		m_menu.closeFolder();
	}

	// スタートボタンを押した場合、フォルダを開く または プレイ開始
	if (KeyConfig::Down(KeyConfig::kStart))
	{
		m_menu.decide();
	}
}

void SelectScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_bgAnim.draw();
	m_menu.draw();
}
