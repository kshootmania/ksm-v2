#include "select_menu.hpp"
#include <cassert>
#include "kson/kson.hpp"
#include "menu_item/select_menu_song_item.hpp"
#include "menu_item/select_menu_all_folder_item.hpp"
#include "menu_item/select_menu_dir_folder_item.hpp"
#include "menu_item/select_menu_sub_dir_section_item.hpp"

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
		return Scaled(Vec2{ 0.0, Cos(Math::HalfPi * timeSec / kShakeDurationSec) * kShakeHeight * (direction == kUp ? -1 : 1) });
	}
}

bool SelectMenu::openDirectory(FilePathView directoryPath, PlaySeYN playSe)
{
	using Unicode::FromUTF8;

	if (playSe)
	{
		m_folderSelectSe.play();
	}

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

		// ディレクトリの見出し項目を追加
		m_menu.push_back(std::make_unique<SelectMenuDirFolderItem>(IsCurrentFolderYN::Yes, FileSystem::FullPath(directoryPath)));

		// TODO: Insert course items

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

		// サブディレクトリ内の曲の項目を追加
		for (const auto& subDirCandidate : subDirCandidates)
		{
			// サブディレクトリの見出し項目を追加
			m_menu.push_back(std::make_unique<SelectMenuSubDirSectionItem>(FileSystem::FullPath(subDirCandidate))); // TODO: foldername.csvから読み込んだフォルダ名で置換

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

			if (found && i == 0)
			{
				// 現在開いているフォルダは項目タイプkCurrentFolderとして既に追加済みなのでスキップ
				continue;
			}

			const auto& directory = directories[rotatedIdx];
			m_menu.push_back(std::make_unique<SelectMenuDirFolderItem>(IsCurrentFolderYN::No, FileSystem::FullPath(directory)));

			if (rotatedIdx == directories.size() - 1)
			{
				// "All"フォルダの項目を追加
				//m_menu.push_back(std::make_unique<SelectMenuAllFolderItem>(IsCurrentFolderYN::No));

				// TODO: "Courses"フォルダの項目を追加
			}
		}
	}

	ConfigIni::SetString(ConfigIni::Key::kSelectDirectory, directoryPath);
	ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, 0);

	refreshGraphics(SelectMenuGraphics::kAll);
	refreshSongPreview();

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

void SelectMenu::refreshSongPreview()
{
	if (m_menu.cursorValue() == nullptr)
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
		m_songPreview.requestSongPreview(pChartInfo->previewBGMFilePath(), pChartInfo->previewBGMOffsetSec(), pChartInfo->previewBGMDurationSec(), pChartInfo->previewBGMVolume());
	}
}

SelectMenu::SelectMenu(std::function<void(FilePathView)> fnMoveToPlayScene)
	: m_eventContext
		{
			.fnMoveToPlayScene = [fnMoveToPlayScene](FilePath path) { fnMoveToPlayScene(path); },
			.fnOpenDirectory = [this](FilePath path) { openDirectory(path, PlaySeYN::Yes); },
			.fnCloseFolder = [this]() { closeFolder(PlaySeYN::Yes); },
		}
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
	, m_shakeStopwatch(StartImmediately::No)
{
	// ConfigIniのカーソルの値はopenDirectory内で上書きされるので事前に取得しておく
	const int32 loadedCursor = ConfigIni::GetInt(ConfigIni::Key::kSelectSongIndex);
	const int32 loadedDifficultyIdx = ConfigIni::GetInt(ConfigIni::Key::kSelectDifficulty);

	// 前回開いていたフォルダを復元
	if (openDirectory(ConfigIni::GetString(ConfigIni::Key::kSelectDirectory), PlaySeYN::No))
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

	refreshGraphics(SelectMenuGraphics::kAll);
	refreshSongPreview();
}

SelectMenu::~SelectMenu() = default;

void SelectMenu::update()
{
	m_menu.update();
	if (const int32 deltaCursor = m_menu.deltaCursor(); deltaCursor != 0)
	{
		ConfigIni::SetInt(ConfigIni::Key::kSelectSongIndex, m_menu.cursor());
		m_songSelectSe.play();
		refreshGraphics(deltaCursor > 0 ? SelectMenuGraphics::kCursorDown : SelectMenuGraphics::kCursorUp);
		refreshSongPreview();
	}

	m_difficultyMenu.update();
	if (m_difficultyMenu.deltaCursor() != 0)
	{
		ConfigIni::SetInt(ConfigIni::Key::kSelectDifficulty, m_difficultyMenu.cursor());
		m_difficultySelectSe.play();
		refreshGraphics(SelectMenuGraphics::kAll);
		refreshSongPreview();
	}

	m_songPreview.update();
}

void SelectMenu::draw() const
{
	const Vec2 shakeVec = ShakeVec(m_shakeDirection, m_shakeStopwatch.sF());

	m_graphics.draw(shakeVec);
	m_difficultyMenu.draw(shakeVec);
}

void SelectMenu::decide()
{
	if (m_menu.empty() || m_menu.cursorValue() == nullptr)
	{
		return;
	}

	m_menu.cursorValue()->decide(m_eventContext, m_difficultyMenu.cursor());
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

	refreshGraphics(SelectMenuGraphics::kAll);
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

void SelectMenu::fadeOutSongPreviewForExit(double durationSec)
{
	m_songPreview.fadeOutForExit(durationSec);
}
