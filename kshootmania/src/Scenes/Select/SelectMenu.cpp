#include "SelectMenu.hpp"
#include <cassert>
#include "kson/kson.hpp"
#include "IR/IRHash.hpp"
#include "MenuItem/SelectMenuSongItem.hpp"
#include "MenuItem/SelectMenuAllFolderItem.hpp"
#include "MenuItem/SelectMenuDirFolderItem.hpp"
#include "MenuItem/SelectMenuFavFolderItem.hpp"
#include "MenuItem/SelectMenuCourseItem.hpp"
#include "MenuItem/SelectMenuCoursesFolderItem.hpp"
#include "MenuItem/SelectMenuSubDirSectionItem.hpp"
#include "MenuItem/SelectMenuLevelSectionItem.hpp"
#include "Common/FsUtils.hpp"
#include "Common/Encoding.hpp"
#include "Input/PlatformKey.hpp"
#include "Course/CourseInfo.hpp"

namespace
{
	Array<FilePath> GetSubDirectories(FilePathView path)
	{
		Array<FilePath> directories =
			FileSystem::DirectoryContents(path, Recursive::No)
				.filter(
					[](FilePathView p)
					{
						return FileSystem::IsDirectory(p);
					});

		// フォルダ名(小文字変換)の昇順でソート
		directories.sort_by([](const FilePath& a, const FilePath& b)
		{
			return FsUtils::DirectoryNameByDirectoryPath(a).lowercased() < FsUtils::DirectoryNameByDirectoryPath(b).lowercased();
		});

		return directories;
	}

	// foldername.csvを読み込んでフォルダ名→表示名の対応関係を取得
	// (キー"*"は全フォルダに対して適用される表示名を示す)
	HashTable<String, String> LoadFolderNameTable(FilePathView directoryPath)
	{
		HashTable<String, String> folderNameTable;
		const FilePath csvPath = FileSystem::PathAppend(directoryPath, U"foldername.csv");

		if (!FileSystem::IsFile(csvPath))
		{
			return folderNameTable;
		}

		// ファイルを行ごとに読み込む(BOMに基づいてShift-JISまたはUTF-8として)
		const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(csvPath);
		for (const String& line : lines)
		{
			// カンマで分割
			const Array<String> parts = line.split(U',');
			if (parts.size() < 2)
			{
				continue;
			}

			// 左右のダブルクオートを除去
			auto removeQuotes = [](String str) -> String
			{
				str = str.trimmed();
				if (str.length() >= 2 && str.starts_with(U'"') && str.ends_with(U'"'))
				{
					return str.substr(1, str.length() - 2);
				}
				return str;
			};

			const String folderName = removeQuotes(parts[0]);
			const String displayName = removeQuotes(parts[1]);

			// フォルダ名が空でなければ対応関係に追加
			// (displayNameが空文字列の場合はサブフォルダ見出しの非表示化を表すため、displayNameが空の場合も除外しない)
			if (!folderName.isEmpty())
			{
				folderNameTable[folderName] = displayName;
			}
		}

		return folderNameTable;
	}

	// foldername.csvによる置換後の表示フォルダ名を取得(対応関係がない場合はnone)
	Optional<String> GetDisplayNameFromFolderNameTable(const HashTable<String, String>& folderNameTable, const String& folderName)
	{
		if (const auto it = folderNameTable.find(folderName); it != folderNameTable.end())
		{
			return it->second;
		}

		// "*"による全フォルダ置換があれば使用
		if (const auto it = folderNameTable.find(U"*"); it != folderNameTable.end())
		{
			return it->second;
		}

		return none;
	}

	// お気に入りフォルダの特殊パスのプレフィックス
	constexpr char32 kFavFolderSpecialPathPrefix = U'?';

	// .favファイルのフルパスから特殊パスを生成
	String ToSpecialPath(FilePathView favFilePath)
	{
		const String filename = FileSystem::FileName(favFilePath);
		return kFavFolderSpecialPathPrefix + FsUtils::EliminateExtension(filename);
	}

	// 特殊パスから.favファイルのフルパスを取得
	FilePath FromSpecialPath(StringView specialPath)
	{
		if (specialPath.isEmpty() || specialPath[0] != kFavFolderSpecialPathPrefix)
		{
			return U"";
		}

		const String displayName = String(specialPath.substr(1));
		const FilePath songsDir = FsUtils::SongsDirectoryPath();
		const FilePath favFilePath = FileSystem::PathAppend(songsDir, displayName + U".fav");

		if (FileSystem::IsFile(favFilePath))
		{
			return favFilePath;
		}

		return U"";
	}

	// 特殊パスがお気に入りフォルダを示すかどうか
	bool IsSpecialPathFavorite(StringView specialPath)
	{
		return !specialPath.isEmpty() && specialPath[0] == kFavFolderSpecialPathPrefix;
	}

	// songsディレクトリ配下の.favファイルを列挙
	Array<FilePath> GetSortedFavFiles()
	{
		const FilePath songsDir = FsUtils::SongsDirectoryPath();
		Array<FilePath> favFiles = FileSystem::DirectoryContents(songsDir, Recursive::No)
			.filter([](FilePathView p)
			{
				return FileSystem::IsFile(p) && FileSystem::Extension(p) == U"fav";
			});

		// ファイル名(小文字)でソート
		favFiles.sort_by([](const FilePath& a, const FilePath& b)
		{
			return FileSystem::FileName(a).lowercased() < FileSystem::FileName(b).lowercased();
		});

		return favFiles;
	}

	// .favファイルから楽曲パスの一覧を読み込む
	Array<String> LoadFavFile(FilePathView favFilePath)
	{
		if (!FileSystem::IsFile(favFilePath))
		{
			return {};
		}

		const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(favFilePath);
		Array<String> result;

		for (const String& line : lines)
		{
			const String trimmed = line.trimmed();
			if (trimmed.isEmpty())
			{
				continue;
			}
			result.push_back(trimmed);
		}

		return result;
	}

	std::unique_ptr<ISelectMenuItem> CreateFolderMenuItem(FilePathView folderPath, IsCurrentFolderYN isCurrentFolder)
	{
		// Allフォルダの特殊パスかどうかで項目タイプを判定
		if (folderPath == SelectMenuAllFolderItem::kAllFolderSpecialPath)
		{
			return std::make_unique<SelectMenuAllFolderItem>(isCurrentFolder);
		}
		else if (folderPath == SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath)
		{
			// Coursesフォルダの特殊パス
			return std::make_unique<SelectMenuCoursesFolderItem>(isCurrentFolder);
		}
		else if (IsSpecialPathFavorite(folderPath))
		{
			// お気に入りフォルダの特殊パス
			return std::make_unique<SelectMenuFavFolderItem>(isCurrentFolder, folderPath);
		}
		else
		{
			// 通常のディレクトリ
			return std::make_unique<SelectMenuDirFolderItem>(isCurrentFolder, FileSystem::FullPath(folderPath));
		}
	}

	// ディレクトリ内の.kcoファイルを読み込み、タイトルでソートされたコース項目リストを返す
	Array<std::unique_ptr<SelectMenuCourseItem>> LoadCourseItemsSorted(FilePathView directoryPath)
	{
		Array<std::unique_ptr<SelectMenuCourseItem>> courseItems;

		const Array<FilePath> kcoFiles = FileSystem::DirectoryContents(directoryPath, Recursive::No)
			.filter([](FilePathView p)
			{
				return FileSystem::IsFile(p) && FileSystem::Extension(p) == U"kco";
			});

		if (kcoFiles.isEmpty())
		{
			return courseItems;
		}

		struct CourseItemInfo
		{
			CourseInfo courseInfo;
			String lowercasedTitle;
		};
		Array<CourseItemInfo> tempCourseItems;

		for (const auto& kcoFile : kcoFiles)
		{
			const auto courseInfoOpt = CourseInfo::Load(kcoFile);
			if (!courseInfoOpt)
			{
				Logger << U"[ksm warning] LoadCourseItemsSorted: Failed to load course file (path:'{}')"_fmt(kcoFile);
				continue;
			}

			tempCourseItems.push_back(CourseItemInfo{
				.courseInfo = *courseInfoOpt,
				.lowercasedTitle = courseInfoOpt->title.lowercased(),
			});
		}

		tempCourseItems.sort_by([](const CourseItemInfo& a, const CourseItemInfo& b)
		{
			return a.lowercasedTitle < b.lowercasedTitle;
		});

		for (const auto& item : tempCourseItems)
		{
			courseItems.push_back(std::make_unique<SelectMenuCourseItem>(item.courseInfo));
		}

		return courseItems;
	}
}

bool SelectMenu::openDirectory(FilePathView directoryPath, PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview, SaveToConfigIniYN saveToConfigIni)
{
	if (playSe)
	{
		m_folderSelectSe.play();
	}

	// ソートモードに応じて処理を分岐
	bool result = false;
	if (m_folderState.sortMode == SelectFolderState::SortMode::kLevel)
	{
		result = openDirectoryWithLevelSort(directoryPath);
	}
	else
	{
		result = openDirectoryWithNameSort(directoryPath);
	}

	if (!result)
	{
		return false;
	}

	if (saveToConfigIni)
	{
		// songsフォルダからの相対パスとして保存
		const FilePath fullDirectoryPath = FileSystem::FullPath(directoryPath);
		FilePath relativeDirectoryPath = FsUtils::RelativePathFromSongsDir(fullDirectoryPath);
		// 末尾の/を除去
		if (relativeDirectoryPath.ends_with(U'/'))
		{
			relativeDirectoryPath.pop_back();
		}
		ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, relativeDirectoryPath);
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, 0);
	}

	refreshContentCanvasParams();

	if (refreshSongPreview)
	{
		this->refreshSongPreview();
	}

	return true;
}

bool SelectMenu::openDirectoryWithNameSort(FilePathView directoryPath)
{
	using Unicode::FromUTF8;

	// 曲の項目を挿入する関数
	// (挿入されたかどうかを返す)
	const auto fnInsertSongItem = [this](const String& songDirectory) -> bool
	{
		if (!FileSystem::IsDirectory(songDirectory))
		{
			return false;
		}

		std::unique_ptr<SelectMenuSongItem> item = std::make_unique<SelectMenuSongItem>(songDirectory);
		if (item->chartExists())
		{
			m_menu.push_back(std::move(item));
			return true;
		}
		else
		{
			return false;
		}
	};

	if (!directoryPath.empty())
	{
		if (!FileSystem::IsDirectory(directoryPath))
		{
			// ディレクトリが存在しない場合は何もしない
			return false;
		}

		m_menu.clear();
		m_jacketTextureCache.clear();
		m_iconTextureCache.clear();

		// ディレクトリの見出し項目を追加
		m_menu.push_back(std::make_unique<SelectMenuDirFolderItem>(IsCurrentFolderYN::Yes, FileSystem::FullPath(directoryPath)));

		// コース項目を追加
		{
			auto courseItems = LoadCourseItemsSorted(directoryPath);
			for (auto& courseItem : courseItems)
			{
				m_menu.push_back(std::move(courseItem));
			}
		}

		// 曲の項目を追加
		Array<FilePath> subDirCandidates;
		{
			const Array<FilePath> songDirectories = GetSubDirectories(directoryPath);
			for (const auto& songDirectory : songDirectories)
			{
				// 項目追加
				const bool chartExists = fnInsertSongItem(songDirectory);

				// フォルダ直下に譜面がなかった場合はサブディレクトリの候補に追加
				if (!chartExists)
				{
					subDirCandidates.push_back(songDirectory);
				}
			}
		}

		// foldername.csvを読み込んでフォルダ名の対応関係を取得
		const HashTable<String, String> folderNameTable = LoadFolderNameTable(directoryPath);

		// サブディレクトリ内の曲の項目を追加
		for (const auto& subDirCandidate : subDirCandidates)
		{
			const String folderName = FsUtils::DirectoryNameByDirectoryPath(subDirCandidate);
			const Optional<String> displayName = GetDisplayNameFromFolderNameTable(folderNameTable, folderName);

			// 表示名が空文字列の場合は見出し項目を追加せず、曲だけを追加
			const bool shouldSkipHeading = displayName.has_value() && displayName->isEmpty();

			if (!shouldSkipHeading)
			{
				// サブディレクトリの見出し項目を追加
				m_menu.push_back(std::make_unique<SelectMenuSubDirSectionItem>(FileSystem::FullPath(subDirCandidate), displayName));
			}

			bool chartExists = false;
			const Array<FilePath> songDirectories = GetSubDirectories(subDirCandidate);
			for (const auto& songDirectory : songDirectories)
			{
				// 項目追加
				if (fnInsertSongItem(songDirectory))
				{
					chartExists = true;
				}
			}

			// サブディレクトリ内に譜面が存在しなかった場合は見出し項目を削除
			if (!shouldSkipHeading && !chartExists)
			{
				m_menu.pop_back();
			}
		}

		m_folderState.folderType = SelectFolderState::kDirectory;
		m_folderState.fullPath = FileSystem::FullPath(directoryPath);
	}
	else
	{
		m_menu.clear();
		m_jacketTextureCache.clear();
		m_iconTextureCache.clear();

		m_folderState.folderType = SelectFolderState::kNone;
		m_folderState.fullPath = U"";
	}

	// フォルダ項目を追加
	// (フォルダを開いていない場合、または現在開いていないフォルダを表示する設定の場合のみ)
	if (directoryPath.empty() || ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

void SelectMenu::setCursorAndSave(int32 cursor)
{
	m_menu.setCursor(cursor);
	ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, cursor);
}

void SelectMenu::setCursorToItemByFullPath(FilePathView fullPath)
{
	// 大した数ではないので線形探索
	for (std::size_t i = 0U; i < m_menu.size(); ++i)
	{
		if (fullPath == m_menu[i]->fullPath())
		{
			setCursorAndSave(static_cast<int32>(i));
			break;
		}
	}
}

void SelectMenu::refreshContentCanvasParams()
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 difficultyCursor = m_difficultyMenu.cursor(); // この値は-1にもなり得る
	const int32 difficultyIdx = difficultyCursor >= 0 ? difficultyCursor : m_difficultyMenu.rawCursor();
	m_selectSceneCanvas->setParamValues({
		{ U"difficultyCursorState", U"difficulty{}"_fmt(difficultyIdx) },
		{ U"scrollBarCursorRate", m_menu.cursorRate() },
	});

	// 中央の項目のパラメータを反映
	if (const auto pItem = m_menu.cursorValue().get())
	{
		pItem->setCanvasParamsCenter(m_eventContext, *m_selectSceneCanvas, difficultyIdx);
	}

	// 上の項目のパラメータを反映
	for (int32 i = 0; i < kNumTopItems; ++i)
	{
		if (const auto pItem = m_menu.atCyclic(m_menu.cursor() - kNumTopItems + i).get())
		{
			const int32 topIdx = kNumTopItems - i;
			pItem->setCanvasParamsTopBottom(m_eventContext, *m_selectSceneCanvas, difficultyIdx, U"top{}"_fmt(topIdx));
		}
	}

	// 下の項目のパラメータを反映
	for (int32 i = 0; i < kNumBottomItems; ++i)
	{
		if (const auto pItem = m_menu.atCyclic(m_menu.cursor() + 1 + i).get())
		{
			const int32 bottomIdx = i + 1;
			pItem->setCanvasParamsTopBottom(m_eventContext, *m_selectSceneCanvas, difficultyIdx, U"bottom{}"_fmt(bottomIdx));
		}
	}
}

void SelectMenu::refreshSongPreview()
{
	if (m_menu.empty() || m_menu.cursorValue() == nullptr)
	{
		m_songPreview.requestDefaultBgm();
		return;
	}

	const auto pChartInfo = m_menu.cursorValue()->chartInfoPtr(m_difficultyMenu.cursor());
	if (pChartInfo == nullptr)
	{
		m_songPreview.requestDefaultBgm();
	}
	else
	{
		m_songPreview.requestSongPreview(pChartInfo->previewBGMFilePath(), pChartInfo->previewBGMOffset(), pChartInfo->previewBGMDuration(), pChartInfo->previewBGMVolume());
	}
}

void SelectMenu::playShakeUpTween()
{
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeDown", false);

	// 再度再生できるようにオフにしてからオンにする
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeUp", false);
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeUp", true);
}

void SelectMenu::playShakeDownTween()
{
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeUp", false);

	// 再度再生できるようにオフにしてからオンにする
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeDown", false);
	m_selectSceneCanvas->setTweenActiveByTag(U"shakeDown", true);
}

SelectMenu::SelectMenu(const std::shared_ptr<noco::Canvas>& selectSceneCanvas, std::function<void(FilePathView, MusicGame::IsAutoPlayYN, const Optional<CoursePlayState>&)> fnMoveToPlayScene)
	: m_eventContext
		{
			.fnMoveToPlayScene = [fnMoveToPlayScene](FilePath path, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState) { fnMoveToPlayScene(path, isAutoPlay, courseState); },
			.fnOpenDirectory = [this](FilePath path) { openDirectory(path, PlaySeYN::Yes); },
			.fnOpenAllFolder = [this]() { openAllFolder(PlaySeYN::Yes); },
			.fnOpenFavoriteFolder = [this](FilePath specialPath) { openFavoriteFolder(specialPath, PlaySeYN::Yes); },
			.fnOpenCoursesFolder = [this]() { openCoursesFolder(PlaySeYN::Yes); },
			.fnCloseFolder = [this]() { closeFolder(PlaySeYN::Yes); },
			.fnGetJacketTexture = [this](FilePathView path) -> const Texture& { return getJacketTexture(path); },
			.fnGetIconTexture = [this](FilePathView path) -> const Texture& { return getIconTexture(path); },
			.fnMoveToNextSubDirSection = [this]() { moveToNextSubDirSection(); },
			.fnMoveToPrevSubDirSection = [this]() { moveToPrevSubDirSection(); },
		}
	, m_selectSceneCanvas(selectSceneCanvas)
	, m_menu(
		LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.05,
				.buttonIntervalSecFirst = 0.3,
			},
			.cyclic = IsCyclicMenuYN::Yes,
		})
	, m_difficultyMenu(this)
{
	// ConfigIniからソートモードを読み込み
	const StringView sortModeStr = ConfigIni::GetString(ConfigIni::Key::kSelectSortType, U"");
	if (sortModeStr == U"level")
	{
		m_folderState.sortMode = SelectFolderState::SortMode::kLevel;
	}
	else
	{
		m_folderState.sortMode = SelectFolderState::SortMode::kName;
	}

	// ConfigIniのカーソルの値はopenDirectory内で上書きされるので事前に取得しておく
	const int32 loadedCursor = ConfigIni::GetInt(ConfigIni::Key::kSelectSongIndex);
	const int32 loadedDifficultyIdx = ConfigIni::GetInt(ConfigIni::Key::kSelectDifficulty);

	// 前回開いていたフォルダを復元
	const StringView savedDirectory = ConfigIni::GetString(ConfigIni::Key::kSelectDirectory);
	bool openSuccess = false;

	if (savedDirectory == SelectMenuAllFolderItem::kAllFolderSpecialPath)
	{
		// Allフォルダの場合
		openSuccess = openAllFolder(PlaySeYN::No);
	}
	else if (savedDirectory == SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath)
	{
		// Coursesフォルダの場合
		openSuccess = openCoursesFolder(PlaySeYN::No, RefreshSongPreviewYN::No, SaveToConfigIniYN::No);
	}
	else if (IsSpecialPathFavorite(savedDirectory))
	{
		// お気に入りフォルダの場合
		const FilePath favFile = FromSpecialPath(savedDirectory);
		if (!favFile.isEmpty() && FileSystem::IsFile(favFile))
		{
			openSuccess = openFavoriteFolder(
				savedDirectory,
				PlaySeYN::No,
				RefreshSongPreviewYN::No,
				SaveToConfigIniYN::No);
		}
	}
	else if (!savedDirectory.isEmpty())
	{
		// 絶対パスや..を含むパスは受け付けない
		const bool isInvalidPath = savedDirectory.starts_with(U'/') || savedDirectory.starts_with(U'\\') || savedDirectory.includes(U':') || savedDirectory.includes(U"..");

		if (!isInvalidPath)
		{
			// songsフォルダからの相対パスなので結合
			const FilePath fullPath = FileSystem::PathAppend(FsUtils::SongsDirectoryPath(), savedDirectory);
			openSuccess = openDirectory(fullPath, PlaySeYN::No);
		}
	}

	if (openSuccess)
	{
		// 前回選択していたインデックスを復元
		m_menu.setCursor(loadedCursor);
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, loadedCursor);

		// 前回選択していた難易度を復元
		m_difficultyMenu.setCursor(loadedDifficultyIdx);
		ConfigIni::SetInt(ConfigIni::Key::kSelectDifficulty, loadedDifficultyIdx);
	}
	else
	{
		openDirectory(U"", PlaySeYN::No);
	}

	refreshContentCanvasParams();
	refreshSongPreview();
}

SelectMenu::~SelectMenu() = default;

void SelectMenu::update(SongPreviewOnlyYN songPreviewOnly)
{
	if (songPreviewOnly)
	{
		// 楽曲プレビューのみ更新
		m_songPreview.update();
		return;
	}

	m_menu.update();
	if (const int32 deltaCursor = m_menu.deltaCursor(); deltaCursor != 0)
	{
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, m_menu.cursor());
		m_songSelectSe.play();
		if (deltaCursor > 0)
		{
			playShakeDownTween();
		}
		else
		{
			playShakeUpTween();
		}
		refreshContentCanvasParams();
		refreshSongPreview();
	}

	// FX-L + FX-R同時押しでソートモード切り替え
	static bool fxLRPressed = false;
	const bool fxLRPressedNow = KeyConfig::Pressed(kButtonFX_L) && KeyConfig::Pressed(kButtonFX_R);
	if (fxLRPressedNow && !fxLRPressed && isFolderOpen())
	{
		// ソートモードを切り替え
		if (m_folderState.sortMode == SelectFolderState::SortMode::kName)
		{
			m_folderState.sortMode = SelectFolderState::SortMode::kLevel;
			ConfigIni::SetString(ConfigIni::Key::kSelectSortType, U"level");
		}
		else
		{
			m_folderState.sortMode = SelectFolderState::SortMode::kName;
			ConfigIni::SetString(ConfigIni::Key::kSelectSortType, U"");
		}

		// 現在のディレクトリを再読み込み
		reloadCurrentDirectory(RefreshSongPreviewYN::Yes);
	}
	fxLRPressed = fxLRPressedNow;

	// 各種操作と干渉しないようCtrl・Shift・BT-B+C押下時は無視
	const bool btBCPressed = KeyConfig::Pressed(kButtonBT_B) && KeyConfig::Pressed(kButtonBT_C);
	if (!PlatformKey::KeyCommandControl.pressed() && !KeyShift.pressed() && !btBCPressed)
	{
		bool difficultyChanged = false;
		m_difficultyMenu.update(&difficultyChanged);
		if (difficultyChanged)
		{
			ConfigIni::SetInt(ConfigIni::Key::kSelectDifficulty, m_difficultyMenu.cursor());
			m_difficultySelectSe.play();
			refreshContentCanvasParams();
			refreshSongPreview();
		}
	}

	// Homeキーで先頭に移動
	if (KeyHome.down() && !m_menu.empty())
	{
		jumpToFirst();
	}

	// Endキーで末尾に移動
	if (KeyEnd.down() && !m_menu.empty())
	{
		jumpToLast();
	}

	m_songPreview.update();
}

void SelectMenu::decide()
{
	if (m_menu.empty() || m_menu.cursorValue() == nullptr)
	{
		return;
	}

	m_menu.cursorValue()->decide(m_eventContext, m_difficultyMenu.cursor());
}

void SelectMenu::decideAutoPlay()
{
	if (m_menu.empty() || m_menu.cursorValue() == nullptr)
	{
		return;
	}

	m_menu.cursorValue()->decideAutoPlay(m_eventContext, m_difficultyMenu.cursor());
}

bool SelectMenu::isFolderOpen() const
{
	return m_folderState.folderType != SelectFolderState::kNone;
}

void SelectMenu::closeFolder(PlaySeYN playSe)
{
	if (playSe)
	{
		m_folderSelectSe.play();
	}

	// 元のパスを取得しておく
	const String originalFullPath = m_folderState.fullPath;

	// ルートディレクトリを開く
	openDirectory(U"", PlaySeYN::No);

	// カーソルを元々開いていたフォルダに合わせる
	setCursorToItemByFullPath(originalFullPath);

	ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, U"");
	ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, m_menu.cursor());

	refreshContentCanvasParams();
	refreshSongPreview();
}

const ISelectMenuItem& SelectMenu::cursorMenuItem() const
{
	return *m_menu.cursorValue();
}

bool SelectMenu::empty() const
{
	return m_menu.empty();
}

void SelectMenu::fadeOutSongPreviewForExit(Duration duration)
{
	m_songPreview.fadeOutForExit(duration);
}

void SelectMenu::reloadCurrentDirectory(RefreshSongPreviewYN refreshSongPreview)
{
	// 現在選択中の譜面ファイルパスと難易度を保持
	FilePath currentChartFilePath;
	int32 currentDifficulty = m_difficultyMenu.rawCursor();
	int32 currentCursorIndex = m_menu.cursor();
	if (!m_menu.empty() && m_menu.cursorValue() != nullptr)
	{
		const auto* pItem = m_menu.cursorValue().get();
		const auto pChartInfo = pItem->chartInfoPtr(currentDifficulty);
		if (pChartInfo != nullptr)
		{
			currentChartFilePath = pChartInfo->chartFilePath();
		}
		else if (pItem->isSubFolderHeading())
		{
			// 見出し項目の場合、次の曲項目を探してその譜面ファイルパスを使用
			for (std::size_t i = static_cast<std::size_t>(currentCursorIndex) + 1; i < m_menu.size(); ++i)
			{
				const auto& nextItem = m_menu[i];
				if (nextItem == nullptr)
				{
					continue;
				}

				// フォルダ項目や見出し項目はスキップ
				if (nextItem->isFolder() || nextItem->isSubFolderHeading())
				{
					continue;
				}

				// 存在する難易度を探す
				for (int32 difficultyIdx = 0; difficultyIdx < kNumDifficulties; ++difficultyIdx)
				{
					const auto pNextChartInfo = nextItem->chartInfoPtr(difficultyIdx);
					if (pNextChartInfo != nullptr)
					{
						currentChartFilePath = pNextChartInfo->chartFilePath();
						break;
					}
				}

				if (!currentChartFilePath.isEmpty())
				{
					break;
				}
			}
		}
	}

	const FilePath currentDirectory = m_folderState.fullPath;
	const SelectFolderState::FolderType currentFolderType = m_folderState.folderType;

	if (currentFolderType == SelectFolderState::kAll)
	{
		openAllFolder(PlaySeYN::No, RefreshSongPreviewYN::No, SaveToConfigIniYN::No);
	}
	else if (currentFolderType == SelectFolderState::kFavorite)
	{
		openFavoriteFolder(currentDirectory, PlaySeYN::No, RefreshSongPreviewYN::No, SaveToConfigIniYN::No);
	}
	else if (currentFolderType == SelectFolderState::kCourses)
	{
		openCoursesFolder(PlaySeYN::No, RefreshSongPreviewYN::No, SaveToConfigIniYN::No);
	}
	else
	{
		openDirectory(currentDirectory, PlaySeYN::No, RefreshSongPreviewYN::No, SaveToConfigIniYN::No);
	}

	// 譜面ファイルパスから項目を探してフォーカスを復元
	if (!currentChartFilePath.isEmpty())
	{
		bool found = false;
		for (std::size_t i = 0U; i < m_menu.size(); ++i)
		{
			const auto& pItem = m_menu[i];
			if (pItem == nullptr)
			{
				continue;
			}

			// 全難易度から一致する譜面を探す
			for (int32 difficultyIdx = 0; difficultyIdx < kNumDifficulties; ++difficultyIdx)
			{
				const auto pChartInfo = pItem->chartInfoPtr(difficultyIdx);
				if (pChartInfo != nullptr && pChartInfo->chartFilePath() == currentChartFilePath)
				{
					m_menu.setCursor(static_cast<int32>(i));
					m_difficultyMenu.setCursor(difficultyIdx);
					found = true;
					break;
				}
			}

			if (found)
			{
				break;
			}
		}

		// 譜面ファイルパスで復元できなかった場合はカーソルインデックスで復元
		if (!found && currentCursorIndex < static_cast<int32>(m_menu.size()))
		{
			m_menu.setCursor(currentCursorIndex);
		}
	}
	else if (currentCursorIndex < static_cast<int32>(m_menu.size()))
	{
		// 譜面ファイルパスがない場合もカーソルインデックスで復元
		m_menu.setCursor(currentCursorIndex);
	}

	refreshContentCanvasParams();
	if (refreshSongPreview)
	{
		this->refreshSongPreview();
	}
}

void SelectMenu::refreshHighScoreDisplay()
{
	refreshContentCanvasParams();
}

const Texture& SelectMenu::getJacketTexture(FilePathView filePath)
{
	if (auto it = m_jacketTextureCache.find(filePath); it != m_jacketTextureCache.end())
	{
		return it->second;
	}

	Texture texture;
	FilePath actualFilePath{ filePath };

	// 拡張子なしの場合はimgs/jacket内の画像を使用
	if (FileSystem::Extension(actualFilePath).isEmpty())
	{
		const String baseName = FileSystem::BaseName(actualFilePath);
		if (!baseName.isEmpty())
		{
			actualFilePath = FileSystem::PathAppend(U"imgs/jacket", baseName + U".jpg");
		}
	}

	if (FileSystem::IsFile(actualFilePath))
	{
		texture = Texture{ actualFilePath };
	}
	else
	{
		Logger << U"[ksm warning] SelectMenu::getJacketTexture: Jacket image file not found (filePath:'{}')"_fmt(filePath);
	}

	return m_jacketTextureCache.emplace(filePath, std::move(texture)).first->second;
}

const Texture& SelectMenu::getIconTexture(FilePathView filePath)
{
	if (auto it = m_iconTextureCache.find(filePath); it != m_iconTextureCache.end())
	{
		return it->second;
	}

	Texture texture;
	FilePath actualFilePath{ filePath };

	// 拡張子なしの場合はimgs/icon内の画像を使用
	if (FileSystem::Extension(actualFilePath).isEmpty())
	{
		const String baseName = FileSystem::BaseName(actualFilePath);
		if (!baseName.isEmpty())
		{
			actualFilePath = FileSystem::PathAppend(U"imgs/icon", baseName + U".png");
		}
	}

	if (FileSystem::IsFile(actualFilePath))
	{
		texture = Texture{ actualFilePath };
	}
	else
	{
		Logger << U"[ksm warning] SelectMenu::getIconTexture: Icon image file not found (filePath:'{}')"_fmt(filePath);
	}

	return m_iconTextureCache.emplace(filePath, std::move(texture)).first->second;
}

void SelectMenu::moveToNextSubDirSection()
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 currentCursor = m_menu.cursor();

	// 現在のカーソル位置の次から検索
	for (std::size_t i = static_cast<std::size_t>(currentCursor) + 1; i < m_menu.size(); ++i)
	{
		const auto& pItem = m_menu[i];

		if (pItem == nullptr)
		{
			continue;
		}

		// サブフォルダ見出し項目またはレベル見出し項目かどうかをチェック
		if (pItem->isSubFolderHeading())
		{
			// 次の見出し項目が見つかった
			setCursorAndSave(static_cast<int32>(i));
			m_folderSelectSe.play();
			refreshContentCanvasParams();
			refreshSongPreview();
			return;
		}
	}

	// 見出し項目が見つからなかった場合、先頭のフォルダ項目(index=0)に移動
	setCursorAndSave(0);
	m_folderSelectSe.play();
	refreshContentCanvasParams();
	refreshSongPreview();
}

void SelectMenu::moveToPrevSubDirSection()
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 currentCursor = m_menu.cursor();

	// 現在のカーソル位置の前から逆向きに検索
	for (int32 i = currentCursor - 1; i >= 0; --i)
	{
		const auto& pItem = m_menu[static_cast<std::size_t>(i)];

		if (pItem == nullptr)
		{
			continue;
		}

		// サブフォルダ見出し項目またはレベル見出し項目かどうかをチェック
		if (pItem->isSubFolderHeading())
		{
			// 前の見出し項目が見つかった
			setCursorAndSave(i);
			m_folderSelectSe.play();
			refreshContentCanvasParams();
			refreshSongPreview();
			return;
		}
	}

	// 見出し項目が見つからなかった場合、先頭のフォルダ項目(index=0)に移動
	setCursorAndSave(0);
	m_folderSelectSe.play();
	refreshContentCanvasParams();
	refreshSongPreview();
}

void SelectMenu::jumpToAlphabetItem(char32 letter)
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 currentCursor = m_menu.cursor();
	const auto& currentItem = m_menu.cursorValue();

	// 走査開始位置を決定
	int32 scanStartIdx = 0;
	if (currentItem && !currentItem->isSubFolderHeading())
	{
		// 現在の項目が通常の楽曲の場合、上に遡ってセクション見出しを探す
		for (int32 i = currentCursor - 1; i >= 0; --i)
		{
			if (m_menu[static_cast<std::size_t>(i)]->isSubFolderHeading())
			{
				scanStartIdx = i;
				break;
			}
		}
	}
	else if (currentItem && currentItem->isSubFolderHeading())
	{
		// 現在がセクション見出しの場合、その次から開始
		scanStartIdx = currentCursor + 1;
	}

	// 現在のセクション内を走査(次のセクション見出しまで)
	for (std::size_t i = static_cast<std::size_t>(scanStartIdx); i < m_menu.size(); ++i)
	{
		const auto& pItem = m_menu[i];

		if (pItem == nullptr)
		{
			continue;
		}

		// 次のセクション見出しに到達したら終了
		if (i > static_cast<std::size_t>(scanStartIdx) && pItem->isSubFolderHeading())
		{
			break;
		}

		// フォルダ項目はスキップ
		if (pItem->isFolder())
		{
			continue;
		}

		// fullPath()から楽曲フォルダ名を抽出して先頭文字を比較
		const String folderName = FsUtils::DirectoryNameByDirectoryPath(pItem->fullPath());

		if (!folderName.isEmpty() &&
			ToLower(folderName[0]) == ToLower(letter))
		{
			setCursorAndSave(static_cast<int32>(i));
			m_songSelectSe.play();
			refreshContentCanvasParams();
			refreshSongPreview();
			return;
		}
	}
}

void SelectMenu::jumpToNextAlphabet()
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 currentCursor = m_menu.cursor();
	const auto& currentItem = m_menu.cursorValue();

	// 現在の項目がフォルダまたはセクション見出しの場合は何もしない
	if (!currentItem || currentItem->isFolder() || currentItem->isSubFolderHeading())
	{
		return;
	}

	// 現在の楽曲の先頭文字を取得
	const String currentFolderName = FsUtils::DirectoryNameByDirectoryPath(currentItem->fullPath());
	if (currentFolderName.isEmpty())
	{
		return;
	}
	const char32 currentFirstChar = ToLower(currentFolderName[0]);

	// 走査範囲の終了位置を決定(次のセクション見出しまで)
	std::size_t scanEndIdx = m_menu.size();
	for (std::size_t i = static_cast<std::size_t>(currentCursor) + 1; i < m_menu.size(); ++i)
	{
		const auto& pItem = m_menu[i];
		if (pItem && pItem->isSubFolderHeading())
		{
			scanEndIdx = i;
			break;
		}
	}

	// 現在の項目の次から走査
	for (std::size_t i = static_cast<std::size_t>(currentCursor) + 1; i < scanEndIdx; ++i)
	{
		const auto& pItem = m_menu[i];

		if (pItem == nullptr || pItem->isFolder())
		{
			continue;
		}

		const String folderName = FsUtils::DirectoryNameByDirectoryPath(pItem->fullPath());
		if (folderName.isEmpty())
		{
			continue;
		}

		const char32 firstChar = ToLower(folderName[0]);

		if (firstChar != currentFirstChar)
		{
			// 異なる先頭文字が見つかった
			setCursorAndSave(static_cast<int32>(i));
			m_songSelectSe.play();
			refreshContentCanvasParams();
			refreshSongPreview();
			return;
		}
	}
}

void SelectMenu::jumpToPrevAlphabet()
{
	if (m_menu.empty())
	{
		return;
	}

	const int32 currentCursor = m_menu.cursor();
	const auto& currentItem = m_menu.cursorValue();

	// 現在の項目がフォルダまたはセクション見出しの場合は何もしない
	if (!currentItem || currentItem->isFolder() || currentItem->isSubFolderHeading())
	{
		return;
	}

	// 現在の楽曲の先頭文字を取得
	const String currentFolderName = FsUtils::DirectoryNameByDirectoryPath(currentItem->fullPath());
	if (currentFolderName.isEmpty())
	{
		return;
	}
	const char32 currentFirstChar = ToLower(currentFolderName[0]);

	// 走査範囲の開始位置を決定(前のセクション見出しまで)
	int32 scanStartIdx = 0;
	for (int32 i = currentCursor - 1; i >= 0; --i)
	{
		const auto& pItem = m_menu[static_cast<std::size_t>(i)];
		if (pItem && pItem->isSubFolderHeading())
		{
			scanStartIdx = i + 1;
			break;
		}
	}

	// 現在の項目の前から逆向きに走査
	int32 targetIdx = -1;
	char32 targetFirstChar = U'\0';
	for (int32 i = currentCursor - 1; i >= scanStartIdx; --i)
	{
		const auto& pItem = m_menu[static_cast<std::size_t>(i)];

		if (pItem == nullptr || pItem->isFolder())
		{
			continue;
		}

		const String folderName = FsUtils::DirectoryNameByDirectoryPath(pItem->fullPath());
		if (folderName.isEmpty())
		{
			continue;
		}

		const char32 firstChar = ToLower(folderName[0]);

		if (firstChar != currentFirstChar)
		{
			// 異なる先頭文字が見つかった
			if (targetFirstChar == U'\0' || firstChar == targetFirstChar)
			{
				// 最初に見つかった異なる文字、またはその文字と同じ文字
				targetFirstChar = firstChar;
				targetIdx = i;
			}
			else
			{
				// さらに異なる文字が見つかったので、前回の位置で停止
				break;
			}
		}
	}

	if (targetIdx >= 0)
	{
		setCursorAndSave(targetIdx);
		m_songSelectSe.play();
		refreshContentCanvasParams();
		refreshSongPreview();
	}
}

bool SelectMenu::openDirectoryWithLevelSort(FilePathView directoryPath)
{
	if (!directoryPath.empty())
	{
		if (!FileSystem::IsDirectory(directoryPath))
		{
			// ディレクトリが存在しない場合は何もしない
			return false;
		}

		m_menu.clear();
		m_jacketTextureCache.clear();
		m_iconTextureCache.clear();

		// ディレクトリの見出し項目を追加
		m_menu.push_back(std::make_unique<SelectMenuDirFolderItem>(IsCurrentFolderYN::Yes, FileSystem::FullPath(directoryPath)));

		// コース項目を追加
		{
			auto courseItems = LoadCourseItemsSorted(directoryPath);
			for (auto& courseItem : courseItems)
			{
				m_menu.push_back(std::move(courseItem));
			}
		}

		// レベルごとに譜面を分類
		struct ChartFileInfo
		{
			FilePath filePath;
			int32 difficultyIdx;
			String songDirectoryName;
		};
		std::array<Array<ChartFileInfo>, kNumLevels> chartsByLevel;

		// 譜面ファイルを処理してchartsByLevelに追加する関数
		const auto fnProcessChartFiles = [&chartsByLevel](const FilePath& directory, const Array<FilePath>& chartFiles)
		{
			for (const auto& chartFile : chartFiles)
			{
				auto chartInfo = std::make_unique<SelectChartInfo>(chartFile);
				if (chartInfo->hasError())
				{
					Logger << U"[ksm warning] SelectMenu::openDirectoryWithLevelSort: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(chartInfo->errorString(), chartFile);
					continue;
				}

				const int32 level = chartInfo->level();
				const int32 difficultyIdx = chartInfo->difficultyIdx();

				if (level < 1 || level > 20)
				{
					Logger << U"[ksm warning] SelectMenu::openDirectoryWithLevelSort: Level out of range (level:{}, chartFilePath:'{}')"_fmt(level, chartFile);
					continue;
				}

				if (difficultyIdx < 0 || difficultyIdx >= kNumDifficulties)
				{
					Logger << U"[ksm warning] SelectMenu::openDirectoryWithLevelSort: Difficulty index out of range (difficultyIdx:{}, chartFilePath:'{}')"_fmt(difficultyIdx, chartFile);
					continue;
				}

				chartsByLevel[level - 1].push_back(ChartFileInfo{
					.filePath = chartFile,
					.difficultyIdx = difficultyIdx,
					.songDirectoryName = FsUtils::DirectoryNameByDirectoryPath(directory).lowercased(),
				});
			}
		};

		// 全サブディレクトリを走査して譜面を収集
		const Array<FilePath> songDirectories = GetSubDirectories(directoryPath);
		for (const auto& songDirectory : songDirectories)
		{
			const Array<FilePath> subDirs = GetSubDirectories(songDirectory);
			for (const auto& subDir : subDirs)
			{
				// サブディレクトリ内の譜面ファイルを走査
				const Array<FilePath> chartFiles = FsUtils::GetChartFilePathsPreferringKson(subDir);
				fnProcessChartFiles(subDir, chartFiles);
			}

			// songDirectory直下の譜面ファイルも走査
			const Array<FilePath> chartFiles = FsUtils::GetChartFilePathsPreferringKson(songDirectory);
			fnProcessChartFiles(songDirectory, chartFiles);
		}

		// レベルごとに見出しと譜面項目を追加
		for (int32 level = 1; level <= 20; ++level)
		{
			Array<ChartFileInfo>& charts = chartsByLevel[level - 1];
			if (charts.isEmpty())
			{
				continue;
			}

			// フォルダ名(小文字変換)の昇順でソート
			charts.sort_by([](const ChartFileInfo& a, const ChartFileInfo& b)
			{
				return a.songDirectoryName < b.songDirectoryName;
			});

			// レベル見出し項目を追加
			m_menu.push_back(std::make_unique<SelectMenuLevelSectionItem>(level));

			// 譜面項目を追加
			for (const auto& chartInfo : charts)
			{
				auto item = std::make_unique<SelectMenuSongItem>(chartInfo.filePath);
				if (item->chartExists())
				{
					m_menu.push_back(std::move(item));
				}
			}
		}

		m_folderState.folderType = SelectFolderState::kDirectory;
		m_folderState.fullPath = FileSystem::FullPath(directoryPath);
	}
	else
	{
		m_menu.clear();
		m_jacketTextureCache.clear();
		m_iconTextureCache.clear();

		m_folderState.folderType = SelectFolderState::kNone;
		m_folderState.fullPath = U"";
	}

	// フォルダ項目を追加
	// (フォルダを開いていない場合、または現在開いていないフォルダを表示する設定の場合のみ)
	if (directoryPath.empty() || ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

bool SelectMenu::openAllFolder(PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview, SaveToConfigIniYN saveToConfigIni)
{
	if (playSe)
	{
		m_folderSelectSe.play();
	}

	// ソートモードに応じて処理を分岐
	bool result = false;
	if (m_folderState.sortMode == SelectFolderState::SortMode::kLevel)
	{
		result = openAllFolderWithLevelSort();
	}
	else
	{
		result = openAllFolderWithNameSort();
	}

	if (!result)
	{
		return false;
	}

	if (saveToConfigIni)
	{
		ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, SelectMenuAllFolderItem::kAllFolderSpecialPath);
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, 0);
	}

	refreshContentCanvasParams();

	if (refreshSongPreview)
	{
		this->refreshSongPreview();
	}

	return true;
}

bool SelectMenu::openAllFolderWithNameSort()
{
	using Unicode::FromUTF8;

	// 曲の項目を挿入する関数
	// (挿入されたかどうかを返す)
	const auto fnInsertSongItem = [this](const String& songDirectory) -> bool
	{
		if (!FileSystem::IsDirectory(songDirectory))
		{
			return false;
		}

		std::unique_ptr<SelectMenuSongItem> item = std::make_unique<SelectMenuSongItem>(songDirectory);
		if (item->chartExists())
		{
			m_menu.push_back(std::move(item));
			return true;
		}
		else
		{
			return false;
		}
	};

	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// Allフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuAllFolderItem>(IsCurrentFolderYN::Yes));

	// TODO: Insert course items

	// 全フォルダの楽曲を収集
	const Array<FilePath> searchPaths = {
		FsUtils::SongsDirectoryPath(), // TODO: 設定可能にする
	};

	// 全フォルダを取得
	Array<FilePath> allFolderDirectories;
	for (const auto& path : searchPaths)
	{
		allFolderDirectories.append(GetSubDirectories(path).map([](FilePathView p) { return FileSystem::FullPath(p); }));
	}

	// 全楽曲フォルダを収集(フォルダ名でソート)
	struct SongDirectoryInfo
	{
		FilePath path;
		String lowercasedDirectoryName;
	};
	Array<SongDirectoryInfo> allSongDirectories;

	for (const auto& folderDirectory : allFolderDirectories)
	{
		const Array<FilePath> songDirectories = GetSubDirectories(folderDirectory);
		for (const auto& songDirectory : songDirectories)
		{
			// サブディレクトリがあるかチェック
			const Array<FilePath> subDirs = GetSubDirectories(songDirectory);
			if (!subDirs.isEmpty())
			{
				// サブディレクトリ内の楽曲フォルダを追加
				for (const auto& subDir : subDirs)
				{
					allSongDirectories.push_back(SongDirectoryInfo{
						.path = subDir,
						.lowercasedDirectoryName = FsUtils::DirectoryNameByDirectoryPath(subDir).lowercased(),
					});
				}
			}
			else
			{
				// サブディレクトリがない場合は直接追加
				allSongDirectories.push_back(SongDirectoryInfo{
					.path = songDirectory,
					.lowercasedDirectoryName = FsUtils::DirectoryNameByDirectoryPath(songDirectory).lowercased(),
				});
			}
		}
	}

	// フォルダ名(小文字変換)の昇順でソート
	allSongDirectories.sort_by([](const SongDirectoryInfo& a, const SongDirectoryInfo& b)
	{
		return a.lowercasedDirectoryName < b.lowercasedDirectoryName;
	});

	// 曲の項目を追加
	for (const auto& songDirInfo : allSongDirectories)
	{
		fnInsertSongItem(songDirInfo.path);
	}

	m_folderState.folderType = SelectFolderState::kAll;
	m_folderState.fullPath = SelectMenuAllFolderItem::kAllFolderSpecialPath;

	// フォルダ項目を追加
	// (現在開いていないフォルダを表示する設定の場合のみ)
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsSimple();
	}

	return true;
}

bool SelectMenu::openAllFolderWithLevelSort()
{
	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// Allフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuAllFolderItem>(IsCurrentFolderYN::Yes));

	// TODO: Insert course items

	// レベルごとに譜面を分類
	struct ChartFileInfo
	{
		FilePath filePath;
		int32 difficultyIdx;
		String songDirectoryName;
	};
	std::array<Array<ChartFileInfo>, kNumLevels> chartsByLevel;

	// 譜面ファイルを処理してchartsByLevelに追加する関数
	const auto fnProcessChartFiles = [&chartsByLevel](const FilePath& directory, const Array<FilePath>& chartFiles)
	{
		for (const auto& chartFile : chartFiles)
		{
			auto chartInfo = std::make_unique<SelectChartInfo>(chartFile);
			if (chartInfo->hasError())
			{
				Logger << U"[ksm warning] SelectMenu::openAllFolderWithLevelSort: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(chartInfo->errorString(), chartFile);
				continue;
			}

			const int32 level = chartInfo->level();
			const int32 difficultyIdx = chartInfo->difficultyIdx();

			if (level < 1 || level > 20)
			{
				Logger << U"[ksm warning] SelectMenu::openAllFolderWithLevelSort: Level out of range (level:{}, chartFilePath:'{}')"_fmt(level, chartFile);
				continue;
			}

			if (difficultyIdx < 0 || difficultyIdx >= kNumDifficulties)
			{
				Logger << U"[ksm warning] SelectMenu::openAllFolderWithLevelSort: Difficulty index out of range (difficultyIdx:{}, chartFilePath:'{}')"_fmt(difficultyIdx, chartFile);
				continue;
			}

			chartsByLevel[level - 1].push_back(ChartFileInfo{
				.filePath = chartFile,
				.difficultyIdx = difficultyIdx,
				.songDirectoryName = FsUtils::DirectoryNameByDirectoryPath(directory).lowercased(),
			});
		}
	};

	// 全フォルダの楽曲を収集
	const Array<FilePath> searchPaths = {
		FsUtils::SongsDirectoryPath(), // TODO: 設定可能にする
	};

	// 全フォルダを取得
	Array<FilePath> allFolderDirectories;
	for (const auto& path : searchPaths)
	{
		allFolderDirectories.append(GetSubDirectories(path).map([](FilePathView p) { return FileSystem::FullPath(p); }));
	}

	// 全サブディレクトリを走査して譜面を収集
	for (const auto& folderDirectory : allFolderDirectories)
	{
		const Array<FilePath> songDirectories = GetSubDirectories(folderDirectory);
		for (const auto& songDirectory : songDirectories)
		{
			const Array<FilePath> subDirs = GetSubDirectories(songDirectory);
			for (const auto& subDir : subDirs)
			{
				// サブディレクトリ内の譜面ファイルを走査
				const Array<FilePath> chartFiles = FsUtils::GetChartFilePathsPreferringKson(subDir);
				fnProcessChartFiles(subDir, chartFiles);
			}

			// songDirectory直下の譜面ファイルも走査
			const Array<FilePath> chartFiles = FsUtils::GetChartFilePathsPreferringKson(songDirectory);
			fnProcessChartFiles(songDirectory, chartFiles);
		}
	}

	// レベルごとに見出しと譜面項目を追加
	for (int32 level = 1; level <= 20; ++level)
	{
		Array<ChartFileInfo>& charts = chartsByLevel[level - 1];
		if (charts.isEmpty())
		{
			continue;
		}

		// フォルダ名(小文字変換)の昇順でソート
		charts.sort_by([](const ChartFileInfo& a, const ChartFileInfo& b)
		{
			return a.songDirectoryName < b.songDirectoryName;
		});

		// レベル見出し項目を追加
		m_menu.push_back(std::make_unique<SelectMenuLevelSectionItem>(level));

		// 譜面項目を追加
		for (const auto& chartInfo : charts)
		{
			auto item = std::make_unique<SelectMenuSongItem>(chartInfo.filePath);
			if (item->chartExists())
			{
				m_menu.push_back(std::move(item));
			}
		}
	}

	m_folderState.folderType = SelectFolderState::kAll;
	m_folderState.fullPath = SelectMenuAllFolderItem::kAllFolderSpecialPath;

	// フォルダ項目を追加
	// (現在開いていないフォルダを表示する設定の場合のみ)
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsSimple();
	}

	return true;
}

bool SelectMenu::openFavoriteFolder(FilePathView specialPath, PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview, SaveToConfigIniYN saveToConfigIni)
{
	if (playSe)
	{
		m_folderSelectSe.play();
	}

	// ソートモードに応じて処理を分岐
	bool result = false;
	if (m_folderState.sortMode == SelectFolderState::SortMode::kLevel)
	{
		result = openFavoriteFolderWithLevelSort(specialPath);
	}
	else
	{
		result = openFavoriteFolderWithNameSort(specialPath);
	}

	if (!result)
	{
		return false;
	}

	if (saveToConfigIni)
	{
		ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, specialPath);
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, 0);
	}

	refreshContentCanvasParams();

	if (refreshSongPreview)
	{
		this->refreshSongPreview();
	}

	return true;
}

bool SelectMenu::openFavoriteFolderWithNameSort(FilePathView specialPath)
{
	const FilePath favFilePath = FromSpecialPath(specialPath);
	if (favFilePath.isEmpty())
	{
		return false;
	}

	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// お気に入りフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuFavFolderItem>(IsCurrentFolderYN::Yes, specialPath));

	// .favファイルから楽曲パスを読み込み
	const Array<String> songPaths = LoadFavFile(favFilePath);
	const FilePath songsDir = FsUtils::SongsDirectoryPath();

	// パス情報を収集
	struct PathInfo
	{
		FilePath path;
		String lowercasedName;
	};
	Array<PathInfo> pathInfos;

	for (const String& relativePath : songPaths)
	{
		const FilePath fullPath = FileSystem::PathAppend(songsDir, relativePath);

		if (FileSystem::IsDirectory(fullPath))
		{
			pathInfos.push_back(PathInfo{
				.path = fullPath,
				.lowercasedName = FsUtils::DirectoryNameByDirectoryPath(fullPath).lowercased(),
			});
		}
		else if (FileSystem::IsFile(fullPath) && FsUtils::HasChartExtension(fullPath))
		{
			pathInfos.push_back(PathInfo{
				.path = fullPath,
				.lowercasedName = FileSystem::FileName(fullPath).lowercased(),
			});
		}
		else
		{
			Logger << U"[ksm warning] SelectMenu::openFavoriteFolderWithNameSort: Favorite folder entry does not exist (path:'{}')"_fmt(fullPath);
		}
	}

	// 名前(小文字変換)の昇順でソート
	pathInfos.sort_by([](const PathInfo& a, const PathInfo& b)
	{
		return a.lowercasedName < b.lowercasedName;
	});

	// 項目を追加
	for (const auto& pathInfo : pathInfos)
	{
		std::unique_ptr<SelectMenuSongItem> item = std::make_unique<SelectMenuSongItem>(pathInfo.path);
		if (item->chartExists())
		{
			m_menu.push_back(std::move(item));
		}
	}

	m_folderState.folderType = SelectFolderState::kFavorite;
	m_folderState.fullPath = specialPath;

	// フォルダ項目を追加
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

bool SelectMenu::openFavoriteFolderWithLevelSort(FilePathView specialPath)
{
	const FilePath favFilePath = FromSpecialPath(specialPath);
	if (favFilePath.isEmpty())
	{
		return false;
	}

	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// お気に入りフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuFavFolderItem>(IsCurrentFolderYN::Yes, specialPath));

	// レベルごとに譜面を分類
	constexpr int32 kNumLevels = 20;
	struct ChartFileInfo
	{
		FilePath filePath;
		String songDirectoryName;
	};
	std::array<Array<ChartFileInfo>, kNumLevels> chartsByLevel;

	// .favファイルから楽曲パスを読み込み
	const Array<String> songPaths = LoadFavFile(favFilePath);
	const FilePath songsDir = FsUtils::SongsDirectoryPath();

	// 各楽曲フォルダまたは譜面ファイル(KSH/KSON)から項目を生成
	Array<std::unique_ptr<SelectMenuSongItem>> songItems;
	for (const String& relativePath : songPaths)
	{
		const FilePath fullPath = FileSystem::PathAppend(songsDir, relativePath);

		if (FileSystem::IsDirectory(fullPath) || (FileSystem::IsFile(fullPath) && FsUtils::HasChartExtension(fullPath)))
		{
			auto item = std::make_unique<SelectMenuSongItem>(fullPath);
			if (item->chartExists())
			{
				songItems.push_back(std::move(item));
			}
		}
		else
		{
			Logger << U"[ksm warning] SelectMenu::openFavoriteFolderWithLevelSort: Favorite folder entry does not exist (path:'{}')"_fmt(fullPath);
		}
	}

	// 各項目から譜面情報を取得してレベル別に分類
	for (const auto& item : songItems)
	{
		const FilePath itemPath{ item->fullPath() };
		const String songName = item->isSingleChartItem() ? FileSystem::FileName(itemPath) : FsUtils::DirectoryNameByDirectoryPath(itemPath);

		for (int32 diffIdx = 0; diffIdx < kNumDifficulties; ++diffIdx)
		{
			const auto pChartInfo = item->chartInfoPtr(diffIdx, FallbackForSingleChartYN::No);
			if (pChartInfo == nullptr)
			{
				continue;
			}

			const int32 level = pChartInfo->level();
			if (level < 1 || level > 20)
			{
				Logger << U"[ksm warning] SelectMenu::openFavoriteFolderWithLevelSort: Level out of range (level:{}, chartFilePath:'{}')"_fmt(level, pChartInfo->chartFilePath());
				continue;
			}

			chartsByLevel[level - 1].push_back(ChartFileInfo{
				.filePath = FilePath{ pChartInfo->chartFilePath() },
				.songDirectoryName = songName,
			});
		}
	}

	// レベルごとに見出しと譜面項目を追加
	for (int32 level = 1; level <= kNumLevels; ++level)
	{
		const auto& charts = chartsByLevel[level - 1];
		if (charts.isEmpty())
		{
			continue;
		}

		// レベル見出し項目を追加
		m_menu.push_back(std::make_unique<SelectMenuLevelSectionItem>(level));

		// 曲名(小文字)でソート
		Array<ChartFileInfo> sortedCharts = charts;
		sortedCharts.sort_by([](const ChartFileInfo& a, const ChartFileInfo& b)
		{
			return a.songDirectoryName.lowercased() < b.songDirectoryName.lowercased();
		});

		// 譜面項目を追加
		for (const auto& chartInfo : sortedCharts)
		{
			auto item = std::make_unique<SelectMenuSongItem>(chartInfo.filePath);
			if (item->chartExists())
			{
				m_menu.push_back(std::move(item));
			}
		}
	}

	m_folderState.folderType = SelectFolderState::kFavorite;
	m_folderState.fullPath = specialPath;

	// フォルダ項目を追加
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

bool SelectMenu::openCoursesFolder(PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview, SaveToConfigIniYN saveToConfigIni)
{
	if (playSe)
	{
		m_folderSelectSe.play();
	}

	// ソートモードに応じて処理を分岐
	bool result = false;
	if (m_folderState.sortMode == SelectFolderState::SortMode::kLevel)
	{
		result = openCoursesFolderWithLevelSort();
	}
	else
	{
		result = openCoursesFolderWithNameSort();
	}

	if (!result)
	{
		return false;
	}

	if (saveToConfigIni)
	{
		ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath);
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, 0);
	}

	refreshContentCanvasParams();

	if (refreshSongPreview)
	{
		this->refreshSongPreview();
	}

	return true;
}

bool SelectMenu::openCoursesFolderWithNameSort()
{
	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// コースフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuCoursesFolderItem>(IsCurrentFolderYN::Yes));

	const FilePath coursesDir = FsUtils::CoursesDirectoryPath();
	if (!FileSystem::IsDirectory(coursesDir))
	{
		Logger << U"[ksm warning] SelectMenu::openCoursesFolderWithNameSort: courses directory does not exist";
		m_folderState.folderType = SelectFolderState::kCourses;
		m_folderState.fullPath = SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath;
		return true;
	}

	// コース項目を追加
	{
		auto courseItems = LoadCourseItemsSorted(coursesDir);
		for (auto& courseItem : courseItems)
		{
			m_menu.push_back(std::move(courseItem));
		}
	}

	m_folderState.folderType = SelectFolderState::kCourses;
	m_folderState.fullPath = SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath;

	// フォルダ項目を追加
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

bool SelectMenu::openCoursesFolderWithLevelSort()
{
	m_menu.clear();
	m_jacketTextureCache.clear();
	m_iconTextureCache.clear();

	// コースフォルダの見出し項目を追加
	m_menu.push_back(std::make_unique<SelectMenuCoursesFolderItem>(IsCurrentFolderYN::Yes));

	const FilePath coursesDir = FsUtils::CoursesDirectoryPath();
	if (!FileSystem::IsDirectory(coursesDir))
	{
		Logger << U"[ksm warning] SelectMenu::openCoursesFolderWithLevelSort: courses directory does not exist";
		m_folderState.folderType = SelectFolderState::kCourses;
		m_folderState.fullPath = SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath;
		return true;
	}

	// コース項目を追加
	{
		auto courseItems = LoadCourseItemsSorted(coursesDir);
		for (auto& courseItem : courseItems)
		{
			m_menu.push_back(std::move(courseItem));
		}
	}

	m_folderState.folderType = SelectFolderState::kCourses;
	m_folderState.fullPath = SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath;

	// フォルダ項目を追加
	if (ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		addOtherFolderItemsRotated();
	}

	return true;
}

Array<FilePath> SelectMenu::getSortedFolderPaths() const
{
	const Array<FilePath> searchPaths = {
		FsUtils::SongsDirectoryPath(), // TODO: 設定可能にする
	};

	// "All"フォルダを追加
	Array<FilePath> folderPaths;
	const bool hideAllFolder = ConfigIni::GetBool(ConfigIni::Key::kHideAllFolder, false);
	if (!hideAllFolder)
	{
		folderPaths.emplace_back(SelectMenuAllFolderItem::kAllFolderSpecialPath);
	}

	// お気に入りフォルダを追加
	const Array<FilePath> favFiles = GetSortedFavFiles();
	for (const auto& favFile : favFiles)
	{
		folderPaths.emplace_back(ToSpecialPath(favFile));
	}

	// Coursesフォルダを追加
	const FilePath coursesDir = FsUtils::CoursesDirectoryPath();
	if (FileSystem::IsDirectory(coursesDir))
	{
		folderPaths.emplace_back(SelectMenuCoursesFolderItem::kCoursesFolderSpecialPath);
	}

	// 通常ディレクトリを取得してソート
	Array<FilePath> normalDirectories;
	for (const auto& path : searchPaths)
	{
		normalDirectories.append(GetSubDirectories(path).map([](FilePathView p) { return FileSystem::FullPath(p); }));
	}

	// フォルダ名(小文字変換)の昇順でソート
	normalDirectories.sort_by([](const FilePath& a, const FilePath& b)
	{
		return FsUtils::DirectoryNameByDirectoryPath(a).lowercased() < FsUtils::DirectoryNameByDirectoryPath(b).lowercased();
	});

	// ソート済みの通常ディレクトリを追加
	folderPaths.append(normalDirectories);

	return folderPaths;
}

Optional<std::size_t> SelectMenu::findFolderIndex(const Array<FilePath>& folderPaths, FilePathView targetFullPath) const
{
	if (targetFullPath.empty())
	{
		return none;
	}

	const auto itr = std::find(folderPaths.begin(), folderPaths.end(), targetFullPath);
	if (itr != folderPaths.end())
	{
		return static_cast<std::size_t>(std::distance(folderPaths.begin(), itr));
	}

	return none;
}

void SelectMenu::addOtherFolderItemsRotated()
{
	const Array<FilePath> folderPaths = getSortedFolderPaths();

	// 現在開いているフォルダのインデックスを調べる
	// (フォルダを開いている場合は、そのフォルダが先頭になるような順番で項目を追加する)
	const Optional<std::size_t> currentFolderIdxOpt = findFolderIndex(folderPaths, m_folderState.fullPath);
	const std::size_t currentFolderIdx = currentFolderIdxOpt.value_or(0);

	// 項目を追加
	for (std::size_t i = 0; i < folderPaths.size(); ++i)
	{
		const std::size_t rotatedIdx = (i + currentFolderIdx) % folderPaths.size();

		if (currentFolderIdxOpt.has_value() && i == 0)
		{
			// 現在開いているフォルダは項目タイプkCurrentFolderとして既に追加済みなのでスキップ
			continue;
		}

		const auto& folderPath = folderPaths[rotatedIdx];
		m_menu.push_back(CreateFolderMenuItem(folderPath, IsCurrentFolderYN::No));
	}
}

void SelectMenu::addOtherFolderItemsSimple()
{
	const Array<FilePath> folderPaths = getSortedFolderPaths();

	// 項目を追加
	for (const auto& folderPath : folderPaths)
	{
		// 現在開いているフォルダはスキップ
		if (folderPath == m_folderState.fullPath)
		{
			continue;
		}

		m_menu.push_back(CreateFolderMenuItem(folderPath, IsCurrentFolderYN::No));
	}
}

void SelectMenu::jumpToFirst()
{
	if (m_menu.empty())
	{
		return;
	}

	setCursorAndSave(0);
	m_songSelectSe.play();
	refreshContentCanvasParams();
	refreshSongPreview();
}

void SelectMenu::jumpToLast()
{
	if (m_menu.empty())
	{
		return;
	}

	int32 targetIndex = static_cast<int32>(m_menu.size() - 1);

	// フォルダを開いている場合、フォルダ内の最後の項目に移動
	if (isFolderOpen())
	{
		// 後ろから検索して、フォルダ項目でない最初の項目を見つける
		// (他のフォルダ項目をスキップして、曲項目またはサブフォルダ見出し項目の最後を探す)
		for (std::size_t i = m_menu.size() - 1; i >= 1; --i)
		{
			const auto& pItem = m_menu[i];
			if (pItem != nullptr && !pItem->isFolder())
			{
				targetIndex = static_cast<int32>(i);
				break;
			}
		}
	}

	setCursorAndSave(targetIndex);
	m_songSelectSe.play();
	refreshContentCanvasParams();
	refreshSongPreview();
}

double SelectMenu::getCurrentChartStdBPM() const
{
	if (m_menu.empty())
	{
		return kDefaultBPM;
	}

	const SelectChartInfo* chartInfo = m_menu.cursorValue()->chartInfoPtr(m_difficultyMenu.cursor());
	if (chartInfo == nullptr)
	{
		return kDefaultBPM;
	}

	return chartInfo->stdBPMForHispeedTypeChange();
}

Optional<HighScoreInfo> SelectMenu::getCurrentHighScoreInfo() const
{
	if (m_menu.empty())
	{
		return none;
	}

	return m_menu.cursorValue()->highScoreInfo(m_difficultyMenu.cursor());
}

void SelectMenu::showCurrentItemInFileManager()
{
	if (m_menu.empty())
	{
		return;
	}

	// 現在選択中の項目のshowInFileManagerを呼び出す
	m_menu.cursorValue()->showInFileManager(m_difficultyMenu.cursor());
}

void SelectMenu::openCurrentChartIRPage()
{
	if (m_menu.empty())
	{
		return;
	}

	const SelectChartInfo* pChartInfo = m_menu.cursorValue()->chartInfoPtr(m_difficultyMenu.cursor());
	if (pChartInfo == nullptr)
	{
		return;
	}

	const String irHash = IRHash::Calculate(pChartInfo->chartFilePath());
	if (!irHash.isEmpty())
	{
		System::LaunchBrowser(U"https://ir.kshootmania.com/md5/" + irHash);
	}
}

Optional<String> SelectMenu::currentItemRelativePathToCopy() const
{
	if (m_menu.empty())
	{
		return none;
	}

	return m_menu.cursorValue()->relativePathToCopy(m_difficultyMenu.cursor());
}

const SelectFolderState& SelectMenu::folderState() const
{
	return m_folderState;
}
