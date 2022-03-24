#include "select_menu.hpp"
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

bool SelectMenu::openDirectory(FilePathView directoryPath)
{
	if (!directoryPath.empty())
	{
		if (!FileSystem::IsDirectory(directoryPath)) // Note: FileSystem::IsDirectory() checks if the directory exists.
		{
			return false;
		}

		m_items.clear();

		// Insert the directory heading item
		m_items.push_back({
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
					.jacketFilePath = Widen(chartData.meta.jacketFilename),
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

			m_items.push_back({
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
		m_items.clear();

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
				m_items.push_back({
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
			m_items.push_back({
				.itemType = SelectMenuItem::kDirectoryFolder,
				.fullPath = FileSystem::FullPath(directory),
				.info = std::make_unique<SelectMenuFolderItemInfo>(FileSystem::BaseName(directory)),
			});
		}
	}

	return true;
}

SelectMenu::SelectMenu()
	: m_menu(nullptr)
{
	if (!openDirectory(ConfigIni::GetString(ConfigIni::Key::kSelectDirectory)))
	{
		openDirectory(U"");
	}

	// TODO: Delete this debug code
	Print << openDirectory(U"songs/K-Shoot MANIA");
	for (const auto& item : m_items)
	{
		{
			auto pInfo = dynamic_cast<SelectMenuSongItemInfo*>(item.info.get());
			if (pInfo)
			{
				String s;
				for (int i = 0; i < 4; ++i)
				{
					s += pInfo->chartInfos[i].has_value() ? Format(pInfo->chartInfos[i]->title, U",") : U"x,";
				}
				Print << s;
				continue;
			}
		}

		{
			auto pInfo = dynamic_cast<SelectMenuFolderItemInfo*>(item.info.get());
			if (pInfo)
			{
				Print << pInfo->displayName;
				continue;
			}
		}

		if (item.itemType == SelectMenuItem::kAllFolder)
		{
			Print << U"All";
		}
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
