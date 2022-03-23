#include "select_menu.hpp"
#include "ksh/io/ksh_io.hpp"

namespace
{
	String Widen(std::u8string_view str)
	{
		return Unicode::FromUTF8(ksh::UnU8(str));
	}
}

bool SelectMenu::openDirectory(FilePathView directoryPath)
{
	if (!FileSystem::IsDirectory(directoryPath)) // Note: FileSystem::IsDirectory() checks if the directory exists.
	{
		return false;
	}

	m_items.clear();

	const Array<FilePath> songDirectories = FileSystem::DirectoryContents(directoryPath);
	for (const auto& songDirectory : songDirectories)
	{
		if (!FileSystem::IsDirectory(songDirectory))
		{
			continue;
		}

		std::array<Optional<SelectMenuSongItemChartInfo>, kNumDifficulties> chartInfos;
		const Array<FilePath> chartFiles = FileSystem::DirectoryContents(songDirectory);
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

	return true;
}

SelectMenu::SelectMenu()
	: m_menu(nullptr)
{
	Print << openDirectory(ConfigIni::GetString(ConfigIni::Key::kSelectDirectory));
}

bool SelectMenu::isFolderOpen() const
{
	return m_folderState.folderType != SelectFolderState::kNone;
}

void SelectMenu::closeFolder()
{
	m_folderState.folderType = SelectFolderState::kNone;
}
