#include "select_menu.hpp"
#include <cassert>

#include "ui/menu_helper.hpp"
#include "kson/io/ksh_io.hpp"

namespace
{
	Array<FilePath> GetSubDirectories(FilePathView path)
	{
		return
			FileSystem::DirectoryContents(path, Recursive::No)
				.filter(
					[](FilePathView p)
					{
						return FileSystem::IsDirectory(p);
					});
	}

	Vec2 ShakeVec(SelectMenuShakeDirection direction, double timeSec)
	{
		constexpr double kShakeHeight = 2.0;
		constexpr double kShakeDurationSec = 0.04;
		using enum SelectMenuShakeDirection;
		if ((direction != kUp && direction != kDown) || timeSec < 0.0 || kShakeDurationSec < timeSec)
		{
			return Vec2::Zero();
		}
		return ScreenUtils::Scaled(Vec2{ 0.0, Cos(Math::HalfPi * timeSec / kShakeDurationSec) * kShakeHeight * (direction == kUp ? -1 : 1) });
	}
}

void SelectMenu::decideSongItem()
{
	assert(!m_menu.empty());
	assert(m_menu.cursorValue().itemType == SelectMenuItem::kSong);

	// 選択されている譜面の情報を取得
	const auto pChartInfo = cursorChartInfoPtr();
	if (pChartInfo == nullptr) [[unlikely]]
	{
		return;
	}

	// プレイ開始
	m_moveToPlaySceneFunc(pChartInfo->chartFilePath);
}

void SelectMenu::decideDirectoryFolderItem()
{
	assert(!m_menu.empty());
	assert(m_menu.cursorValue().itemType == SelectMenuItem::kDirectoryFolder);

	// フォルダを開く
	// Note: openDirectory()の実行中にm_menu.cursorValue().fullPathの中身は破棄されるので、ここでは新しいFilePathとしてコピーしてから渡している
	const FilePath directoryPath = m_menu.cursorValue().fullPath;
	openDirectory(directoryPath);
}

bool SelectMenu::openDirectory(FilePathView directoryPath)
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

		std::array<Optional<SelectMenuSongItemChartInfo>, kNumDifficulties> chartInfos;
		const Array<FilePath> chartFiles = FileSystem::DirectoryContents(songDirectory, Recursive::No);
		bool chartExists = false;
		for (const auto& chartFile : chartFiles)
		{
			if (FileSystem::Extension(chartFile) != kKSHExtension) // Note: FileSystem::Extension() always returns lowercase.
			{
				continue;
			}

			const kson::MetaChartData chartData = kson::LoadKSHMetaChartData(chartFile.narrow());
			if (chartData.error != kson::Error::None)
			{
				Logger << U"[SelectMenu] KSH Loading Error (" << static_cast<int32>(chartData.error) << U"): " << chartFile;
				continue;
			}

			int32 difficultyIdx = chartData.meta.difficulty.idx;
			if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
			{
				// 未知の難易度の場合は一番右の難易度にする
				difficultyIdx = kNumDifficulties - 1;
			}

			if (chartInfos[difficultyIdx].has_value()) [[unlikely]]
			{
				Logger << U"[SelectMenu] Skip duplication (difficultyIdx:" << difficultyIdx << U"): " << chartFile;
				continue;
			}

			chartInfos[difficultyIdx] = SelectMenuSongItemChartInfo{
				.title = FromUTF8(chartData.meta.title),
				.artist = FromUTF8(chartData.meta.artist),
				.jacketFilePath = FileSystem::FullPath(FileSystem::ParentPath(chartFile) + FromUTF8(chartData.meta.jacketFilename)),
				.jacketAuthor = FromUTF8(chartData.meta.jacketAuthor),
				.chartFilePath = FileSystem::FullPath(chartFile),
				.chartAuthor = FromUTF8(chartData.meta.chartAuthor),
				.level = chartData.meta.level,
				.previewBGMFilePath = FileSystem::FullPath(FileSystem::ParentPath(chartFile) + FromUTF8(chartData.audio.bgm.filename)),
				.previewBGMOffsetSec = chartData.audio.bgm.preview.offset / 1000.0,
				.previewBGMDurationSec = chartData.audio.bgm.preview.duration / 1000.0,
				.iconFilePath = U""/*TODO*/,
				.information = FromUTF8(chartData.meta.information),
				.highScoreInfo = HighScoreInfo{}/*TODO*/,
			};
			chartExists = true;
		}

		if (chartExists)
		{
			m_menu.push_back({
				.itemType = SelectMenuItem::kSong,
				.fullPath = FileSystem::FullPath(songDirectory),
				.info = std::make_unique<SelectMenuSongItemInfo>(chartInfos),
			});
		}

		return chartExists;
	};

	if (!directoryPath.empty())
	{
		if (!FileSystem::IsDirectory(directoryPath))
		{
			// ディレクトリが存在しない場合は何もしない
			return false;
		}

		m_menu.clear();

		// ディレクトリの見出し項目を追加
		m_menu.push_back({
			.itemType = SelectMenuItem::kCurrentFolder,
			.fullPath = FileSystem::FullPath(directoryPath),
			.info = std::make_unique<SelectMenuFolderItemInfo>(FileSystem::FileName(directoryPath)),
		});

		// TODO: Insert course items

		// 曲の項目を追加
		Array<FilePath> subDirCandidates;
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

		// サブディレクトリ内の曲の項目を追加
		for (const auto& subDirCandidate : subDirCandidates)
		{
			// サブディレクトリの見出し項目を追加
			m_menu.push_back({
				.itemType = SelectMenuItem::kSubDir,
				.fullPath = FileSystem::FullPath(subDirCandidate),
				.info = std::make_unique<SelectMenuSectionItemInfo>(FileSystem::FileName(subDirCandidate)), // TODO: foldername.csvから読み込んだフォルダ名で置換
			});

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
			if (!chartExists)
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

		m_folderState.folderType = SelectFolderState::kNone;
		m_folderState.fullPath = U"";
	}

	// フォルダ項目を追加
	// (フォルダを開いていない場合、または現在開いていないフォルダを表示する設定の場合のみ)
	if (directoryPath.empty() || ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		const Array<FilePath> searchPaths = {
			U"songs", // TODO: 設定可能にする
		};

		Array<FilePath> directories;
		for (const auto& path : searchPaths)
		{
			directories.append(GetSubDirectories(path).map([](FilePathView p) { return FileSystem::FullPath(p); }));
		}

		// フォルダを開いている場合は、そのフォルダが先頭になるような順番で項目を追加する必要があるので、現在開いているフォルダのインデックスを調べる
		std::size_t currentDirectoryIdx = 0;
		bool found = false;
		if (!directoryPath.empty())
		{
			const auto itr = std::find(directories.begin(), directories.end(), m_folderState.fullPath);
			if (itr != directories.end())
			{
				currentDirectoryIdx = static_cast<std::size_t>(std::distance(directories.begin(), itr));
				found = true;
			}
		}

		// 項目を追加
		for (std::size_t i = 0; i < directories.size(); ++i)
		{
			const std::size_t rotatedIdx = (i + currentDirectoryIdx) % directories.size();
			if (rotatedIdx == 0)
			{
				// "All"フォルダの項目を追加
				m_menu.push_back({
					.itemType = SelectMenuItem::kAllFolder,
				});

				// TODO: "Courses"フォルダの項目を追加
			}

			if (found && i == 0)
			{
				// 現在開いているフォルダは項目タイプkCurrentFolderとして既に追加済みなのでスキップ
				continue;
			}

			const auto& directory = directories[rotatedIdx];
			m_menu.push_back({
				.itemType = SelectMenuItem::kDirectoryFolder,
				.fullPath = FileSystem::FullPath(directory),
				.info = std::make_unique<SelectMenuFolderItemInfo>(FileSystem::FileName(directory)),
			});
		}
	}

	refreshGraphics(SelectMenuGraphics::kAll);

	return true;
}

void SelectMenu::refreshGraphics(SelectMenuGraphics::RefreshType type)
{
	const int32 difficultyCursor = m_difficultyMenu.cursor(); // could be -1
	const int32 difficultyIdx = (difficultyCursor >= 0) ? difficultyCursor : m_difficultyMenu.rawCursor();
	m_graphics.refresh(m_menu, difficultyIdx, type);
	switch (type)
	{
	case SelectMenuGraphics::kCursorUp:
		m_shakeDirection = SelectMenuShakeDirection::kUp;
		break;

	case SelectMenuGraphics::kCursorDown:
		m_shakeDirection = SelectMenuShakeDirection::kDown;
		break;

	default:
		m_shakeDirection = SelectMenuShakeDirection::kUnspecified;
		break;
	};
	m_shakeStopwatch.restart();
}

SelectMenu::SelectMenu(std::function<void(FilePathView)> moveToPlaySceneFunc)
	: m_menu(MenuHelper::MakeArrayWithVerticalMenu<SelectMenuItem>(MenuHelper::ButtonFlags::kArrowOrLaser, IsCyclicMenu::Yes, 0.05, 0.3))
	, m_difficultyMenu(this)
	, m_shakeStopwatch(StartImmediately::No)
	, m_moveToPlaySceneFunc(std::move(moveToPlaySceneFunc))
	, m_debugFont(12)
{
	if (!openDirectory(ConfigIni::GetString(ConfigIni::Key::kSelectDirectory)))
	{
		openDirectory(U"");
	}

	// TODO: Delete this debug code
	openDirectory(U"songs/K-Shoot MANIA");
}

void SelectMenu::update()
{
	m_menu.update();
	if (m_menu.isCursorChanged())
	{
		refreshGraphics(m_menu.isCursorIncremented() ? SelectMenuGraphics::kCursorDown : SelectMenuGraphics::kCursorUp);
	}

	m_difficultyMenu.update();
	if (m_difficultyMenu.isCursorChanged())
	{
		refreshGraphics(SelectMenuGraphics::kAll);
	}

	// TODO: Delete this debug code
	m_debugStr.clear();
	const auto pCursorItem = m_menu.empty() ? nullptr : &m_menu.cursorValue();
	for (const auto& item : m_menu)
	{
		if (&item == pCursorItem)
		{
			m_debugStr += U"> ";
		}
		else
		{
			m_debugStr += U"  ";
		}

		{
			const auto pInfo = dynamic_cast<SelectMenuSongItemInfo*>(item.info.get());
			if (pInfo)
			{
				for (int i = 0; i < 4; ++i)
				{
					m_debugStr += pInfo->chartInfos[i].has_value() ? Format(pInfo->chartInfos[i]->title, U",") : U"x,";
				}
			}
		}

		{
			const auto pInfo = dynamic_cast<SelectMenuFolderItemInfo*>(item.info.get());
			if (pInfo)
			{
				m_debugStr += pInfo->displayName;
			}
		}

		if (item.itemType == SelectMenuItem::kAllFolder)
		{
			m_debugStr += U"All";
		}

		m_debugStr += '\n';
	}
	m_debugStr += Format(m_difficultyMenu.cursor());
}

void SelectMenu::draw() const
{
	const Vec2 shakeVec = ShakeVec(m_shakeDirection, m_shakeStopwatch.sF());

	m_graphics.draw(shakeVec);
	m_difficultyMenu.draw(shakeVec);

	// TODO: Delete this debug code
	//m_debugFont(m_debugStr).draw(Vec2{ 100, 100 });
}

void SelectMenu::decide()
{
	if (m_menu.empty())
	{
		return;
	}

	switch (m_menu.cursorValue().itemType)
	{
	case SelectMenuItem::kSong:
		decideSongItem();
		break;

	case SelectMenuItem::kDirectoryFolder:
		decideDirectoryFolderItem();
		break;

	default:
		Print << U"Not implemented!";
		break;
	}
}

bool SelectMenu::isFolderOpen() const
{
	return m_folderState.folderType != SelectFolderState::kNone;
}

void SelectMenu::closeFolder()
{
	m_folderState.folderType = SelectFolderState::kNone;
}

const SelectMenuItem& SelectMenu::cursorMenuItem() const
{
	return m_menu.cursorValue();
}

const SelectMenuSongItemChartInfo* SelectMenu::cursorChartInfoPtr() const
{
	// 項目タイプがkSongでなければ譜面情報なし
	const SelectMenuItem& item = m_menu.cursorValue();
	if (item.itemType != SelectMenuItem::kSong)
	{
		return nullptr;
	}

	// 何らかの理由でSelectMenuItem::infoがSelectMenuSongItemInfo*にキャストできなければ譜面情報なし
	// (通常はキャストできるはず)
	const SelectMenuSongItemInfo* const pSongInfo = dynamic_cast<const SelectMenuSongItemInfo*>(item.info.get());
	if (pSongInfo == nullptr) [[unlikely]]
	{
		return nullptr;
	}

	// 選択中の曲にカーソルの難易度が存在しなければ譜面情報なし
	// (通常は存在するはず)
	const int32 cursor = m_difficultyMenu.cursor();
	assert(0 <= cursor && cursor < pSongInfo->chartInfos.size());
	if (!pSongInfo->chartInfos[cursor].has_value()) [[unlikely]]
	{
		return nullptr;
	}

	return &*pSongInfo->chartInfos[cursor];
}

bool SelectMenu::empty() const
{
	return m_menu.empty();
}
