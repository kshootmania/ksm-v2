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
#include "UI/SimpleDialog.hpp"
#include "Input/KeyConfig.hpp"
#include "NocoExtensions/NocoUtils.hpp"
#include "SearchInputDialog.hpp"

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
	// 復元用に検索パラメータを保持
	Optional<SelectSceneSearchParams> searchParams;
	if (m_searchPhase == SelectSceneSearchPhase::kResult)
	{
		SelectSceneSearchParams params;
		params.phase = m_searchPhase;
		params.query = m_searchResultQuery;
		params.cursorChartPath = FilePath{ chartFilePath };
		searchParams = std::move(params);
	}

	m_fadeOutColor = Palette::White;
	requestNextScene<PlayPrepareScene>(FilePath{ chartFilePath }, isAutoPlay, courseState, none, searchParams);
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
	m_menu.clearHighScoreCache();
	m_menu.reloadCurrentDirectory(RefreshSongPreviewYN::No, ReloadFromDiskYN::Yes);
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
	else if (m_fxButtonUpDetection.up(kButtonFX_L)) // FX-Lを単体で離した時
	{
		if (m_menu.folderState().sortMode == SelectFolderState::SortMode::kLevel)
		{
			// 前のレベル見出しにジャンプ
			m_menu.moveToPrevSubDirSection();
		}
		else
		{
			// 前のアルファベットの先頭にジャンプ
			m_menu.jumpToPrevAlphabet();
		}
	}
	else if (m_fxButtonUpDetection.up(kButtonFX_R)) // FX-Rを単体で離した時
	{
		if (m_menu.folderState().sortMode == SelectFolderState::SortMode::kLevel)
		{
			// 次のレベル見出しにジャンプ
			m_menu.moveToNextSubDirSection();
		}
		else
		{
			// 次のアルファベットの先頭にジャンプ
			m_menu.jumpToNextAlphabet();
		}
	}
}

void SelectScene::enterSearchInputPhase()
{
	if (m_searchPhase == SelectSceneSearchPhase::kInput)
	{
		return;
	}

	const bool wasNone = m_searchPhase == SelectSceneSearchPhase::kNone;

	if (m_searchResultNode)
	{
		m_searchResultNode->setActive(false);
	}

	// Ctrl+Fと同フレームでBTボタンが押されていた場合にパネル表示が残らないようにする
	m_btOptionPanel.hide();
	m_playStatsPanel.hide();

	const Optional<FilePath> preservedChartPath = m_menu.currentChartFilePath();

	if (wasNone)
	{
		m_menu.enterSearchMode(preservedChartPath);
	}
	else
	{
		m_menu.setSearchPreservedChartPath(preservedChartPath);
	}

	m_searchInputReentryFromResult = !wasNone;
	m_searchPhase = SelectSceneSearchPhase::kInput;

	const String initialQuery = wasNone ? String{} : m_searchResultQuery;
	m_dialogRunner = Co::Play<SearchInputDialog>(
		initialQuery,
		[this](StringView q) { m_menu.setSearchQuery(q); }
	).runScoped([this](const Optional<String>& result) { onSearchInputDialogClosed(result); });
}

void SelectScene::onSearchInputDialogClosed(const Optional<String>& result)
{
	if (m_searchPhase != SelectSceneSearchPhase::kInput)
	{
		return;
	}

	m_menu.setSearchPreservedChartPath(none);

	const bool reentryFromResult = m_searchInputReentryFromResult;
	m_searchInputReentryFromResult = false;

	if (result.has_value())
	{
		m_searchResultQuery = *result;
		m_canvas->setParamValue(U"searchQueryText", m_searchResultQuery);
		if (m_searchResultNode)
		{
			m_searchResultNode->setActive(true);
		}
		m_searchPhase = SelectSceneSearchPhase::kResult;

		// ダイアログ確定のStartボタンのUpで誤って曲決定が走らないよう抑制
		m_ignoreNextStartUp = true;
	}
	else if (reentryFromResult)
	{
		// 検索結果から再度検索入力ダイアログを開いてEscで閉じた時は、復帰先は検索結果とする
		m_menu.setSearchQuery(m_searchResultQuery);
		if (m_searchResultNode)
		{
			m_searchResultNode->setActive(true);
		}
		m_searchPhase = SelectSceneSearchPhase::kResult;
	}
	else
	{
		// 検索入力ダイアログを抜ける
		m_searchPhase = SelectSceneSearchPhase::kNone;
		if (m_searchResultNode)
		{
			m_searchResultNode->setActive(false);
		}
		m_searchResultQuery.clear();
		m_menu.exitSearchMode();
	}
}

void SelectScene::exitSearchMode()
{
	if (m_searchPhase == SelectSceneSearchPhase::kNone)
	{
		return;
	}

	m_searchPhase = SelectSceneSearchPhase::kNone;
	m_searchResultQuery.clear();

	if (m_searchResultNode)
	{
		m_searchResultNode->setActive(false);
	}

	m_menu.exitSearchMode();
}

SelectScene::SelectScene(const Optional<SelectSceneSearchParams>& initialSearchParams)
	: m_folderCloseButton(
		ConfigIni::GetInt(ConfigIni::Key::kSelectCloseFolderKey) == ConfigIni::Value::SelectCloseFolderKey::kBackButton
			? static_cast<Button>(kButtonBack)
			: static_cast<Button>(kButtonBackspace))
	, m_canvas(LoadSelectSceneCanvas())
	, m_menu(
		m_canvas,
		[this](FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlayYN, Optional<CoursePlayState> courseState) { moveToPlayScene(chartFilePath, isAutoPlayYN, courseState); },
		[this](StringView msg) { m_dialogRunner = Co::Play<SimpleDialog>(String{ U"ERROR" }, String{ msg }).runScoped(); },
		[this]() { exitSearchMode(); })
	, m_playerNames(GetPlayerNames())
	, m_fxButtonUpDetection({ KeyShift })
	, m_btOptionPanel(m_canvas)
	, m_playStatsPanel(m_canvas)
{
	AutoMuteAddon::SetEnabled(true);

	// 検索結果表示ノードを取得
	m_searchResultNode = NocoUtils::GetNodeByPath(m_canvas, { U"SearchResult" });

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
		MessageBoxUtils::ShowOK(U"譜面データが見つかりませんでした。", MessageBoxStyle::Warning);
		m_skipFadeout = true;
		requestNextScene<TitleScene>(TitleMenuItem::kStart);
		return;
	}

	// 検索の初期状態を復元
	if (initialSearchParams.has_value() && initialSearchParams->phase != SelectSceneSearchPhase::kNone)
	{
		const String& restoreQuery = initialSearchParams->query;
		const Optional<FilePath>& cursorPath = initialSearchParams->cursorChartPath;

		m_menu.enterSearchMode(cursorPath);
		if (!restoreQuery.isEmpty())
		{
			m_menu.setSearchQuery(restoreQuery);
		}
		if (cursorPath.has_value())
		{
			m_menu.setCursorByChartFilePath(*cursorPath);
		}

		if (initialSearchParams->phase == SelectSceneSearchPhase::kInput)
		{
			m_searchPhase = SelectSceneSearchPhase::kInput;
			m_dialogRunner = Co::Play<SearchInputDialog>(
				restoreQuery,
				[this](StringView q) { m_menu.setSearchQuery(q); }
			).runScoped([this](const Optional<String>& result) { onSearchInputDialogClosed(result); });
		}
		else
		{
			m_searchResultQuery = restoreQuery;
			m_canvas->setParamValue(U"searchQueryText", m_searchResultQuery);
			if (m_searchResultNode)
			{
				m_searchResultNode->setActive(true);
			}
			m_searchPhase = SelectSceneSearchPhase::kResult;
		}
	}
}

void SelectScene::update()
{
	// ダイアログ表示中は必要な更新のみ実行
	if (Co::HasActiveModal())
	{
		m_menu.update(SongPreviewOnlyYN::Yes);
		m_canvas->update(noco::HitTestEnabledYN::No);
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

	// 検索結果表示中はBackキーまたはBackspaceキーで検索モードを終了
	if (m_searchPhase == SelectSceneSearchPhase::kResult && (KeyConfig::Down(kButtonBack) || KeyConfig::Down(kButtonBackspace)))
	{
		exitSearchMode();
		m_canvas->update();
		return;
	}

	// Ctrl+Fで検索入力へ
	if (PlatformKey::KeyCommandControl.pressed() && KeyF.down() && m_menu.isFolderOpen())
	{
		enterSearchInputPhase();
	}

	const bool inSearchMode = m_searchPhase != SelectSceneSearchPhase::kNone;

	// F5で現在のディレクトリのキャッシュを破棄して再読み込み
	if (!inSearchMode && KeyF5.down())
	{
		m_menu.forceReloadCurrentDirectory();
		m_canvas->update();
		return;
	}

	const bool closeFolder = !inSearchMode && m_menu.isFolderOpen() && KeyConfig::Down(m_folderCloseButton/* ← kBackspace・kBackのいずれかが入っている */);

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
				// 削除ダイアログ
				// お気に入り名を取得(例: "?Favorite1" → "Favorite1")
				String favoriteName = m_menu.folderState().fullPath;
				if (favoriteName.starts_with(U'?'))
				{
					favoriteName = favoriteName.substr(1);
				}
				const FilePath songFullPath{ m_menu.cursorMenuItem().fullPath() };
				const String songRelativePath = FsUtils::RelativePathFromSongsDir(songFullPath);

				m_dialogRunner = Co::Play<FavoriteRemoveDialog>().runScoped(
					[this, favoriteName, songRelativePath](const Optional<FavoriteRemoveChoice>& choice)
					{
						if (!choice.has_value())
						{
							// キャンセル時は何もしない
							return;
						}
						if (*choice == FavoriteRemoveChoice::Yes)
						{
							const bool fileRemoved = RemoveFromFavorite(favoriteName, songRelativePath);
							if (fileRemoved)
							{
								const FilePath songsDir = FsUtils::SongsDirectoryPath();
								const FilePath favPath = FileSystem::PathAppend(songsDir, favoriteName + U".fav");
								if (!FileSystem::Exists(favPath))
								{
									m_menu.closeFolder(PlaySeYN::No);
								}
								else
								{
									m_menu.reloadCurrentDirectory(RefreshSongPreviewYN::Yes, ReloadFromDiskYN::Yes);
								}
							}
						}
						m_ignoreNextStartUp = true;
					});
			}
			else
			{
				// 追加ダイアログ
				const FilePath songFullPath{ m_menu.cursorMenuItem().fullPath() };
				FilePath songFolderFullPath = songFullPath;
				if (!FileSystem::IsDirectory(songFullPath))
				{
					songFolderFullPath = FileSystem::ParentPath(songFullPath);
				}
				const String songRelativePath = FsUtils::RelativePathFromSongsDir(songFolderFullPath);

				m_dialogRunner = Co::Play<FavoriteAddDialog>().runScoped(
					[this, songRelativePath](const Optional<int32>& selectedNumber)
					{
						if (!selectedNumber.has_value())
						{
							// キャンセル時は何もしない
							return;
						}

						// .favファイルが新規作成される場合、他フォルダ表示の更新が必要
						const bool needsReloadAfterAdd =
							ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders) &&
							!FileSystem::Exists(GetFavoriteFilePath(*selectedNumber));

						const bool added = AddSongToFavorite(*selectedNumber, songRelativePath);

						if (added && needsReloadAfterAdd)
						{
							m_menu.reloadCurrentDirectory(RefreshSongPreviewYN::No, ReloadFromDiskYN::Yes);
						}
						m_ignoreNextStartUp = true;
					});
			}

			m_startKeyPressStopwatch.reset();
		}
	}
	else if (!startKeyPressed)
	{
		m_startKeyPressStopwatch.reset();
	}
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
