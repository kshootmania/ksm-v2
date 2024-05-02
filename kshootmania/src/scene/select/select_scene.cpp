#include "select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "scene/title/title_scene.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.5s;
}

void SelectScene::moveToPlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
{
	m_fadeOutColor = Palette::White;
	m_menu.fadeOutSongPreviewForExit(kFadeDuration.count());
	requestNextScene<PlayScene>(chartFilePath, isAutoPlay);
}

SelectScene::SelectScene()
	: m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? KeyConfig::kBack
			: KeyConfig::kBackspace)
	, m_menu([this](FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlayYN) { moveToPlayScene(chartFilePath, isAutoPlayYN); })
{
	AutoMuteAddon::SetEnabled(true);
}

void SelectScene::update()
{
	const bool closeFolder = m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* ← kBackspace・kBackのいずれかが入っている */);

	// Backボタン(Escキー)を押した場合、(フォルダを閉じる状況でなければ)タイトル画面へ戻る
	if (!closeFolder && KeyConfig::Down(KeyConfig::kBack))
	{
		m_fadeOutColor = Palette::Black;
		requestNextScene<TitleScene>(TitleMenuItem::kStart);
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

	// オートプレイボタン(F11)を押した場合、オートプレイ開始
	if (KeyConfig::Down(KeyConfig::kAutoPlay))
	{
		m_menu.decideAutoPlay();
	}
}

void SelectScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_bgAnim.draw();
	m_menu.draw();
}

Co::Task<void> SelectScene::fadeIn()
{
	co_await Co::SimpleFadeIn(kFadeDuration);
}

Co::Task<void> SelectScene::fadeOut()
{
	co_await Co::SimpleFadeOut(kFadeDuration, m_fadeOutColor);
}
