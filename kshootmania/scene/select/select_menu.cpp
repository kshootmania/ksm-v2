#include "select_menu.hpp"
#include <cassert>

#include "ui/menu_helper.hpp"
#include "ksh/io/ksh_io.hpp"

namespace
{
	String Widen(std::u8string_view str)
	{
		return Unicode::FromUTF8(ksh::UnU8(str));
	}

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
}

void SelectMenu::decideSongItem()
{
	assert(!m_menu.empty());
	assert(m_menu.cursorValue().itemType == SelectMenuItem::kSong);

	// Get information of the selected chart
	const auto pChartInfo = cursorChartInfoPtr();
	if (pChartInfo == nullptr) [[unlikely]]
	{
		return;
	}

	// TODO: Start playing
	Print << pChartInfo->chartFilePath;
}

void SelectMenu::decideDirectoryFolderItem()
{
	assert(!m_menu.empty());
	assert(m_menu.cursorValue().itemType == SelectMenuItem::kDirectoryFolder);

	// Open folder
	// Note: Here, fullPath is copied to a new FilePath instance because m_menu is cleared within openDirectory() and the original FilePath is destroyed.
	const FilePath directoryPath = m_menu.cursorValue().fullPath;
	openDirectory(directoryPath);
}

bool SelectMenu::openDirectory(FilePathView directoryPath)
{
	if (!directoryPath.empty())
	{
		if (!FileSystem::IsDirectory(directoryPath)) // Note: FileSystem::IsDirectory() checks if the directory exists.
		{
			return false;
		}

		m_menu.clear();

		// Insert the directory heading item
		m_menu.push_back({
			.itemType = SelectMenuItem::kCurrentFolder,
			.fullPath = FileSystem::FullPath(directoryPath),
			.info = std::make_unique<SelectMenuFolderItemInfo>(FileSystem::BaseName(directoryPath)),
		});

		// TODO: Insert course items

		// Insert song items
		const Array<FilePath> songDirectories = GetSubDirectories(directoryPath);
		for (const auto& songDirectory : songDirectories)
		{
			if (!FileSystem::IsDirectory(songDirectory))
			{
				continue;
			}

			std::array<Optional<SelectMenuSongItemChartInfo>, kNumDifficulties> chartInfos;
			const Array<FilePath> chartFiles = FileSystem::DirectoryContents(songDirectory, Recursive::No);
			for (const auto& chartFile : chartFiles)
			{
				if (FileSystem::Extension(chartFile) != kKSHExtension) // Note: FileSystem::Extension() always returns lowercase.
				{
					continue;
				}

				const ksh::MetaChartData chartData = ksh::LoadKSHMetaChartData(chartFile.narrow());
				if (chartData.error != ksh::Error::kNone)
				{
					Logger << U"[SelectMenu] KSH Loading Error (" << static_cast<int32>(chartData.error) << U"): " << chartFile;
					continue;
				}

				int8 difficultyIdx = chartData.meta.difficulty.idx;
				if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
				{
					// Set to rightmost difficulty if unknown difficulty is detected
					difficultyIdx = kNumDifficulties - 1;
				}

				if (chartInfos[difficultyIdx].has_value()) [[unlikely]]
				{
					Logger << U"[SelectMenu] Skip duplication (difficultyIdx:" << difficultyIdx << U"): " << chartFile;
					continue;
				}

				chartInfos[difficultyIdx] = SelectMenuSongItemChartInfo{
					.title = Widen(chartData.meta.title),
					.artist = Widen(chartData.meta.artist),
					.jacketFilePath = FileSystem::FullPath(FileSystem::ParentPath(chartFile) + Widen(chartData.meta.jacketFilename)),
					.jacketAuthor = Widen(chartData.meta.jacketAuthor),
					.chartFilePath = FileSystem::FullPath(chartFile),
					.chartAuthor = Widen(chartData.meta.chartAuthor),
					.level = chartData.meta.level,
					.previewBGMFilePath = Widen(chartData.audio.bgmInfo.previewFilename),
					.previewBGMOffsetSec = chartData.audio.bgmInfo.previewOffsetMs / 1000.0,
					.previewBGMDurationSec = chartData.audio.bgmInfo.previewDurationMs / 1000.0,
					.iconFilePath = U""/*TODO*/,
					.information = Widen(chartData.meta.information),
					.highScoreInfo = HighScoreInfo{}/*TODO*/,
				};
			}

			m_menu.push_back({
				.itemType = SelectMenuItem::kSong,
				.fullPath = FileSystem::FullPath(songDirectory),
				.info = std::make_unique<SelectMenuSongItemInfo>(chartInfos),
			});
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

	if (directoryPath.empty() || ConfigIni::GetBool(ConfigIni::Key::kAlwaysShowOtherFolders))
	{
		const Array<FilePath> searchPaths = {
			U"songs", // TODO: make this configurable
		};

		Array<FilePath> directories;
		for (const auto& path : searchPaths)
		{
			directories.append(GetSubDirectories(path).map([](FilePathView p) { return FileSystem::FullPath(p); }));
		}

		// Find the currently opened directory item
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

		// Insert directory items
		for (std::size_t i = 0; i < directories.size(); ++i)
		{
			const std::size_t rotatedIdx = (i + currentDirectoryIdx) % directories.size();
			if (rotatedIdx == 0)
			{
				// Insert "All" folder item
				m_menu.push_back({
					.itemType = SelectMenuItem::kAllFolder,
				});

				// TODO: Add "Courses" folder item
			}

			if (found && i == 0)
			{
				// Skip the first item because it has already been inserted as kCurrentFolder
				continue;
			}

			const auto& directory = directories[rotatedIdx];
			m_menu.push_back({
				.itemType = SelectMenuItem::kDirectoryFolder,
				.fullPath = FileSystem::FullPath(directory),
				.info = std::make_unique<SelectMenuFolderItemInfo>(FileSystem::BaseName(directory)),
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
}

SelectMenu::SelectMenu()
	: m_menu(MenuHelper::MakeArrayWithVerticalMenu<SelectMenuItem>(MenuHelper::ButtonFlags::kArrowOrLaser, IsCyclicMenu::Yes, 0.05, 0.3))
	, m_difficultyMenu(this)
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
	m_graphics.draw();

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
	// Check if item type is kSong
	const SelectMenuItem& item = m_menu.cursorValue();
	if (item.itemType != SelectMenuItem::kSong)
	{
		return nullptr;
	}

	// Check if SelectMenuItem::info can be casted to SelectMenuSongItemInfo*
	const SelectMenuSongItemInfo* const pSongInfo = dynamic_cast<const SelectMenuSongItemInfo*>(item.info.get());
	if (pSongInfo == nullptr) [[unlikely]]
	{
		return nullptr;
	}

	// Check the song item has the current cursor difficulty
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
