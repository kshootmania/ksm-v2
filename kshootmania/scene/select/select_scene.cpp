#include "select_scene.hpp"
#include "select_assets.hpp"

SelectScene::SelectScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(TextureAsset(SelectTexture::kBG))
	, m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? KeyConfig::kBack
			: KeyConfig::kBackspace)
{
}

void SelectScene::update()
{
	if (m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* <- kBackspace or kBack */))
	{
		// Close folder
		m_menu.closeFolder();
	}
	else if (KeyConfig::Down(KeyConfig::kBack))
	{
		// Back to title screen
		changeScene(SceneName::kTitle, kDefaultTransitionMs);
	}
}

void SelectScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_bgAnim.draw();
}
