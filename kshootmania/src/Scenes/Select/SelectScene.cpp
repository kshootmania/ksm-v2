#include "SelectScene.hpp"
#include "Scenes/PlayPrepare/PlayPrepareScene.hpp"
#include "Scenes/Title/TitleScene.hpp"
#include "Ini/ConfigIni.hpp"
#include "Common/FsUtils.hpp"
#include "Common/Encoding.hpp"
#include "RuntimeConfig.hpp"
#include "Input/PlatformKey.hpp"
#include "MenuItem/ISelectMenuItem.hpp"
#include "SelectChartInfo.hpp"

namespace
{
	constexpr Duration kFadeInDuration = 0.25s;
	constexpr Duration kFadeOutDuration = 0.4s;
	constexpr Duration kStartKeyLongPressDuration = 0.75s;

	FilePath GetSelectSceneUIFilePath()
	{
		return FsUtils::GetResourcePath(U"ui/scene/select.noco");
	}

	std::shared_ptr<noco::Canvas> LoadSelectSceneCanvas()
	{
		const FilePath uiFilePath = GetSelectSceneUIFilePath();
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}

	Array<String> GetPlayerNames()
	{
		Array<String> playerNames;

		for (const auto& dirPath : FileSystem::DirectoryContents(FsUtils::ScoreDirectoryPath(), Recursive::No))
		{
			if (FileSystem::IsDirectory(dirPath))
			{
				playerNames.push_back(FsUtils::DirectoryNameByDirectoryPath(dirPath));
			}
		}

		if (playerNames.empty())
		{
			playerNames.push_back(U"PLAYER");
		}

		playerNames.sort_by([](const String& a, const String& b)
		{
			return a.lowercased() < b.lowercased();
		});

		return playerNames;
	}

	FilePath GetFavoriteFilePath(int32 favoriteNumber)
	{
		const FilePath songsDir = FsUtils::SongsDirectoryPath();
		return FileSystem::PathAppend(songsDir, U"Favorite{}.fav"_fmt(favoriteNumber));
	}

	bool RemoveFromFavorite(StringView favoriteName, StringView songPath)
	{
		const FilePath songsDir = FsUtils::SongsDirectoryPath();
		const FilePath favPath = FileSystem::PathAppend(songsDir, favoriteName + U".fav");

		if (!FileSystem::Exists(favPath))
		{
			return false;
		}

		// 既存の内容を読み込み
		Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(favPath);

		// 削除するパスを正規化
		FilePath songFullPath = FileSystem::PathAppend(songsDir, songPath);
		songFullPath.replace(U"\\", U"/");
		if (songFullPath.ends_with(U'/'))
		{
			songFullPath.pop_back();
		}

		// 該当行を削除
		bool removed = false;
		for (auto it = lines.begin(); it != lines.end(); )
		{
			// .fav内にあるパスを正規化
			FilePath lineFullPath = FileSystem::PathAppend(songsDir, *it);
			lineFullPath.replace(U"\\", U"/");
			if (lineFullPath.ends_with(U'/'))
			{
				lineFullPath.pop_back();
			}

			// 一致する行が見つかれば削除
			if (lineFullPath == songFullPath)
			{
				it = lines.erase(it);
				removed = true;
				break;
			}
			else
			{
				++it;
			}
		}

		if (!removed)
		{
			return false;
		}

		if (lines.isEmpty())
		{
			// 空になった場合は.favファイルごと削除
			FileSystem::Remove(favPath);
			return true;
		}

		// UTF-8 BOM付きで保存
		TextWriter writer(favPath, TextEncoding::UTF8_WITH_BOM);
		for (const auto& line : lines)
		{
			writer.writeln(line);
		}

		return true;
	}
}

void SelectScene::moveToPlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState)
{
	m_fadeOutColor = Palette::White;
	requestNextScene<PlayPrepareScene>(FilePath{ chartFilePath }, isAutoPlay, courseState);
}

void SelectScene::refreshCanvasPlayerName()
{
	const String playerName{ ConfigIni::GetString(ConfigIni::Key::kCurrentPlayer) };
	m_canvas->setParamValue(U"playerName", playerName);
}

void SelectScene::updatePlayerSwitching()
{
	const bool btBCPressed = KeyConfig::Pressed(kButtonBT_B) && KeyConfig::Pressed(kButtonBT_C);
	if (!btBCPressed)
	{
		return;
	}

	const bool leftDown = KeyConfig::Down(kButtonLeft);
	const bool rightDown = KeyConfig::Down(kButtonRight);

	if (!leftDown && !rightDown)
	{
		return;
	}

	const String currentPlayer{ ConfigIni::GetString(ConfigIni::Key::kCurrentPlayer) };

	auto it = std::find(m_playerNames.begin(), m_playerNames.end(), currentPlayer);
	int32 currentIndex = (it != m_playerNames.end()) ? static_cast<int32>(it - m_playerNames.begin()) : 0;

	if (leftDown)
	{
		currentIndex = (currentIndex - 1 + static_cast<int32>(m_playerNames.size())) % static_cast<int32>(m_playerNames.size());
	}
	else if (rightDown)
	{
		currentIndex = (currentIndex + 1) % static_cast<int32>(m_playerNames.size());
	}

	const String newPlayerName = m_playerNames[currentIndex];
	ConfigIni::SetString(ConfigIni::Key::kCurrentPlayer, newPlayerName);
	ConfigIni::Save();

	refreshCanvasPlayerName();
	m_menu.reloadCurrentDirectory();
}

void SelectScene::updateAlphabetJump()
{
	m_fxButtonUpDetection.update();

	// いずれかのBTボタンが押されているかをチェック
	bool btButtonPressed = false;
	for (Button btButton = kButtonBT_A; btButton <= kButtonBT_D; ++btButton)
	{
		if (KeyConfig::Pressed(btButton))
		{
			btButtonPressed = true;
			break;
		}
	}

	// BTボタン押下中 + FX-Lを離した時はリスト先頭へジャンプ
	if (btButtonPressed && m_fxButtonUpDetection.up(kButtonFX_L))
	{
		m_menu.jumpToFirst();
	}
	// BTボタン押下中 + FX-Rを離した時はリスト末尾へジャンプ
	else if (btButtonPressed && m_fxButtonUpDetection.up(kButtonFX_R))
	{
		m_menu.jumpToLast();
	}
	// FX-Lを単独で離した時は前のアルファベットグループにジャンプ
	else if (m_fxButtonUpDetection.up(kButtonFX_L))
	{
		m_menu.jumpToPrevAlphabet();
	}
	// FX-Rを単独で離した時は次のアルファベットグループにジャンプ
	else if (m_fxButtonUpDetection.up(kButtonFX_R))
	{
		m_menu.jumpToNextAlphabet();
	}
}

SelectScene::SelectScene()
	: m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? static_cast<Button>(kButtonBack)
			: static_cast<Button>(kButtonBackspace))
	, m_canvas(LoadSelectSceneCanvas())
	, m_menu(m_canvas, [this](FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlayYN, Optional<CoursePlayState> courseState) { moveToPlayScene(chartFilePath, isAutoPlayYN, courseState); })
	, m_playerNames(GetPlayerNames())
	, m_fxButtonUpDetection({ KeyShift })
	, m_btOptionPanel(m_canvas)
	, m_playStatsPanel(m_canvas)
	, m_favoriteAddDialog(m_canvas)
	, m_favoriteRemoveDialog(m_canvas)
{
	AutoMuteAddon::SetEnabled(true);

	// iniから読み込んだプレイヤー名が空文字列の場合は"PLAYER"に修正
	const String currentPlayer{ ConfigIni::GetString(ConfigIni::Key::kCurrentPlayer) };
	if (currentPlayer.isEmpty())
	{
		ConfigIni::SetString(ConfigIni::Key::kCurrentPlayer, U"PLAYER");
	}

	refreshCanvasPlayerName();

	// 最初からStartボタンが押されている場合は離した時の入力を無視
	if (KeyConfig::Pressed(kButtonStart))
	{
		m_ignoreNextStartUp = true;
	}

	if (m_menu.empty())
	{
		System::MessageBoxOK(U"譜面データが見つかりませんでした。", MessageBoxStyle::Warning);
		m_skipFadeout = true;
		requestNextScene<TitleScene>(TitleMenuItem::kStart);
	}
}

void SelectScene::update()
{
	// ダイアログ表示中はダイアログ更新のみ実行
	if (anyDialogVisible())
	{
		updateDialogs();
		m_canvas->update();
		return;
	}

	// BTオプションパネル更新
	const bool needsDisplayRefresh = m_btOptionPanel.update(m_menu.getCurrentChartStdBPM());
	if (needsDisplayRefresh)
	{
		// ハイスコア表示を更新
		m_menu.refreshHighScoreDisplay();
	}

	// プレイ統計パネル更新
	m_playStatsPanel.update(m_menu.getCurrentHighScoreInfo(), RuntimeConfig::GetGaugeType());

	// いずれかのパネルが表示中かチェック
	const bool anyPanelVisible = m_btOptionPanel.isVisible() || m_playStatsPanel.isVisible();

	// Backキー処理(パネル表示中でも有効にする)
	const bool closeFolder = m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* ← kBackspace・kBackのいずれかが入っている */);

	// BackSpaceキーまたはBackボタン(Escキー)でフォルダを閉じる
	if (closeFolder)
	{
		m_menu.closeFolder(PlaySeYN::No);
	}

	// Backボタン(Escキー)を押した場合、(フォルダを閉じる状況でなければ)タイトル画面へ戻る
	if (!closeFolder && KeyConfig::Down(kButtonBack))
	{
		m_menu.fadeOutSongPreviewForExit(kFadeOutDuration);
		m_fadeOutColor = Palette::Black;
		requestNextScene<TitleScene>(TitleMenuItem::kStart);
		return;
	}

	// Ctrl+C: 選択中の譜面のパスをクリップボードにコピー
	if (PlatformKey::KeyCommandControl.pressed() && KeyC.down())
	{
		const Optional<String> relativePath = m_menu.currentItemRelativePathToCopy();
		if (relativePath.has_value())
		{
			Clipboard::SetText(*relativePath);
		}
	}

	// Ctrl+O: 選択中の項目をエクスプローラで表示
	if (PlatformKey::KeyCommandControl.pressed() && KeyO.down())
	{
		m_menu.showCurrentItemInFileManager();
	}

	// Ctrl+I: 選択中の譜面のIRランキングページをブラウザで開く
	if (PlatformKey::KeyCommandControl.pressed() && KeyI.down())
	{
		m_menu.openCurrentChartIRPage();
	}

	if (anyPanelVisible)
	{
		// パネル表示中は選曲画面の操作を無効化
		// ただし、アルファベット前後ジャンプ(BTを押しながらFX-L/R)とBackキーは有効にする
		updateAlphabetJump();

		// 楽曲プレビューの更新は実行
		m_menu.update(SongPreviewOnlyYN::Yes);

		m_canvas->update();
		return;
	}

	updatePlayerSwitching();

	// Enter長押し検出
	updateStartKeyLongPress();

	// 各種操作と干渉しないようCtrl・Shiftキー押下中は無視
	if (!PlatformKey::KeyCommandControl.pressed() && !KeyShift.pressed())
	{
		m_menu.update();
	}

	// スタートボタンを離した場合、フォルダを開く または プレイ開始
	// Shift+スタートボタンの場合はオートプレイ開始
	if (KeyConfig::Up(kButtonStart))
	{
		if (m_ignoreNextStartUp)
		{
			// ダイアログ決定用に押下したStartボタンが多重反応しないよう無視
			m_ignoreNextStartUp = false;
		}
		else
		{
			if (KeyShift.pressed())
			{
				m_menu.decideAutoPlay();
			}
			else
			{
				m_menu.decide();
			}
		}
	}

	// オートプレイボタン(F11)を押した場合、オートプレイ開始
	if (KeyConfig::Down(kButtonAutoPlay))
	{
		m_menu.decideAutoPlay();
	}

	// Shift+A〜Zでアルファベットジャンプ
	if (KeyShift.pressed())
	{
		if (KeyA.down()) m_menu.jumpToAlphabetItem(U'A');
		else if (KeyB.down()) m_menu.jumpToAlphabetItem(U'B');
		else if (KeyC.down()) m_menu.jumpToAlphabetItem(U'C');
		else if (KeyD.down()) m_menu.jumpToAlphabetItem(U'D');
		else if (KeyE.down()) m_menu.jumpToAlphabetItem(U'E');
		else if (KeyF.down()) m_menu.jumpToAlphabetItem(U'F');
		else if (KeyG.down()) m_menu.jumpToAlphabetItem(U'G');
		else if (KeyH.down()) m_menu.jumpToAlphabetItem(U'H');
		else if (KeyI.down()) m_menu.jumpToAlphabetItem(U'I');
		else if (KeyJ.down()) m_menu.jumpToAlphabetItem(U'J');
		else if (KeyK.down()) m_menu.jumpToAlphabetItem(U'K');
		else if (KeyL.down()) m_menu.jumpToAlphabetItem(U'L');
		else if (KeyM.down()) m_menu.jumpToAlphabetItem(U'M');
		else if (KeyN.down()) m_menu.jumpToAlphabetItem(U'N');
		else if (KeyO.down()) m_menu.jumpToAlphabetItem(U'O');
		else if (KeyP.down()) m_menu.jumpToAlphabetItem(U'P');
		else if (KeyQ.down()) m_menu.jumpToAlphabetItem(U'Q');
		else if (KeyR.down()) m_menu.jumpToAlphabetItem(U'R');
		else if (KeyS.down()) m_menu.jumpToAlphabetItem(U'S');
		else if (KeyT.down()) m_menu.jumpToAlphabetItem(U'T');
		else if (KeyU.down()) m_menu.jumpToAlphabetItem(U'U');
		else if (KeyV.down()) m_menu.jumpToAlphabetItem(U'V');
		else if (KeyW.down()) m_menu.jumpToAlphabetItem(U'W');
		else if (KeyX.down()) m_menu.jumpToAlphabetItem(U'X');
		else if (KeyY.down()) m_menu.jumpToAlphabetItem(U'Y');
		else if (KeyZ.down()) m_menu.jumpToAlphabetItem(U'Z');
	}

	updateAlphabetJump();

	m_canvas->update();
}

void SelectScene::updateStartKeyLongPress()
{
	const bool startKeyPressed = KeyConfig::Pressed(kButtonStart);
	const bool startKeyDown = KeyConfig::Down(kButtonStart);

	if (startKeyDown && !m_ignoreNextStartUp)
	{
		m_startKeyPressStopwatch.restart();
	}

	if (m_startKeyPressStopwatch.isRunning() && startKeyPressed)
	{
		if (m_startKeyPressStopwatch.elapsed() >= kStartKeyLongPressDuration)
		{
			// お気に入り登録可能な項目が選択されているかチェック
			if (m_menu.empty() || !m_menu.cursorMenuItem().isFavoriteRegisterableItemType())
			{
				m_startKeyPressStopwatch.reset();
				return;
			}

			if (m_menu.folderState().folderType == SelectFolderState::kFavorite)
			{
				// 削除ダイアログ(お気に入り内の場合)
				m_favoriteRemoveDialog.show();
			}
			else
			{
				// 追加ダイアログ
				m_favoriteAddDialog.show();
			}

			m_startKeyPressStopwatch.reset();
		}
	}
	else if (!startKeyPressed)
	{
		m_startKeyPressStopwatch.reset();
	}
}

void SelectScene::updateDialogs()
{
	if (m_favoriteAddDialog.isVisible())
	{
		m_favoriteAddDialog.update();

		// Startボタンで決定
		if (KeyConfig::Down(kButtonStart))
		{
			// 楽曲の相対パス
			const FilePath songFullPath{ m_menu.cursorMenuItem().fullPath() };

			// ゲーム上では楽曲単位での登録のみ対応するため、単一譜面の場合は親フォルダを取得
			FilePath songFolderFullPath = songFullPath;
			if (!FileSystem::IsDirectory(songFullPath))
			{
				songFolderFullPath = FileSystem::ParentPath(songFullPath);
			}

			const String songRelativePath = FsUtils::RelativePathFromSongsDir(songFolderFullPath);

			// .favファイルが新規作成される場合、他フォルダ表示の更新が必要
			// (ファイル存在判定するため、お気に入り追加実行より前で判定する必要があるので注意)
			const bool needsReloadAfterAdd =
				ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders) &&
				!FileSystem::Exists(GetFavoriteFilePath(m_favoriteAddDialog.selectedNumber()));

			// お気に入り追加実行
			const bool added = m_favoriteAddDialog.addToFavorite(songRelativePath);

			if (added && needsReloadAfterAdd)
			{
				m_menu.reloadCurrentDirectory();
			}

			m_favoriteAddDialog.hide();
			m_ignoreNextStartUp = true;
		}
		// Backボタンでキャンセル
		else if (KeyConfig::Down(kButtonBack))
		{
			m_favoriteAddDialog.hide();
		}
	}
	else if (m_favoriteRemoveDialog.isVisible())
	{
		m_favoriteRemoveDialog.update();

		// Startボタンで決定
		if (KeyConfig::Down(kButtonStart))
		{
			if (m_favoriteRemoveDialog.selectedChoice() == FavoriteRemoveChoice::Yes)
			{
				// お気に入り名を取得
				// (例: "?Favorite1" → "Favorite1")
				String favoriteName = m_menu.folderState().fullPath;
				if (favoriteName.starts_with(U'?'))
				{
					favoriteName = favoriteName.substr(1);
				}

				// 楽曲の相対パスを取得
				const FilePath songFullPath{ m_menu.cursorMenuItem().fullPath() };
				const String songRelativePath = FsUtils::RelativePathFromSongsDir(songFullPath);

				// お気に入り削除実行
				const bool fileRemoved = RemoveFromFavorite(favoriteName, songRelativePath);

				if (fileRemoved)
				{
					// .favファイルが削除された場合はフォルダを閉じる
					const FilePath songsDir = FsUtils::SongsDirectoryPath();
					const FilePath favPath = FileSystem::PathAppend(songsDir, favoriteName + U".fav");
					if (!FileSystem::Exists(favPath))
					{
						m_menu.closeFolder(PlaySeYN::No);
					}
					else
					{
						// リロード
						m_menu.reloadCurrentDirectory(RefreshSongPreviewYN::Yes);
					}
				}
			}

			m_favoriteRemoveDialog.hide();
			m_ignoreNextStartUp = true;
		}
		// Backボタンでキャンセル
		else if (KeyConfig::Down(kButtonBack))
		{
			m_favoriteRemoveDialog.hide();
		}
	}
}

bool SelectScene::anyDialogVisible() const
{
	return m_favoriteAddDialog.isVisible() || m_favoriteRemoveDialog.isVisible();
}

void SelectScene::draw() const
{
	m_canvas->draw();
}

Co::Task<void> SelectScene::fadeIn()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	co_await Co::ScreenFadeIn(kFadeInDuration);
}

Co::Task<void> SelectScene::fadeOut()
{
	if (m_skipFadeout)
	{
		co_return;
	}

	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	co_await Co::ScreenFadeOut(kFadeOutDuration, m_fadeOutColor);
}
