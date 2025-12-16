#include "FsUtils.hpp"

namespace FsUtils
{
#if defined(__linux__)
	namespace
	{
		// OpenSiv3D 0.6.16 Linux版でModulePathが相対パスになる現象の回避用
		FilePath g_moduleAbsolutePath;
	}

	void InitModulePathForLinux()
	{
		g_moduleAbsolutePath = FileSystem::FullPath(FileSystem::ModulePath());
	}
#endif

	[[nodiscard]]
	FilePath GetFullPathInFolder(SpecialFolder folder, FilePathView relativePath)
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(folder), relativePath);
	}

	[[nodiscard]]
	FilePath AppDataDirectoryPath()
	{
#ifdef __APPLE__
		// ~/Library/Application Support/kshootmania/
		FilePath homeDir = FileSystem::GetFolderPath(SpecialFolder::UserProfile);
		FilePath appSupport = FileSystem::PathAppend(homeDir, U"Library/Application Support");
		return FileSystem::PathAppend(appSupport, U"kshootmania");
#elif defined(__linux__)
		return FileSystem::ParentPath(g_moduleAbsolutePath);
#else
		return FileSystem::ParentPath(FileSystem::ModulePath());
#endif
	}

	[[nodiscard]]
	FilePath ResourceDirectoryPath()
	{
#ifdef __APPLE__
		FilePath modulePath = FileSystem::ModulePath();

		// バンドルとして起動された場合、ModulePathは.appバンドル自体を返す
		if (modulePath.ends_with(U".app"))
		{
			// /path/to/App.app から
			// /path/to/App.app/Contents/Resources/ へ
			return FileSystem::PathAppend(modulePath, U"Contents/Resources");
		}
		else
		{
			// 直接実行ファイルが起動された場合
			// /path/to/App.app/Contents/MacOS/App から
			// /path/to/App.app/Contents/Resources/ へ
			FilePath contentsPath = FileSystem::ParentPath(modulePath);
			return FileSystem::PathAppend(contentsPath, U"Resources");
		}
#else
		return AppDataDirectoryPath();
#endif
	}

	[[nodiscard]]
	FilePath ScoreDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"score");
	}

	[[nodiscard]]
	FilePath SongsDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"songs");
	}

	[[nodiscard]]
	FilePath SongsDefaultDirectoryPath()
	{
		return FileSystem::PathAppend(ResourceDirectoryPath(), U"songs_default");
	}

	[[nodiscard]]
	FilePath CoursesDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"courses");
	}

	[[nodiscard]]
	FilePath CourseScoreDirectoryPath()
	{
		return FileSystem::PathAppend(CoursesDirectoryPath(), U"score");
	}

	[[nodiscard]]
	FilePath GetResourcePath(FilePathView folderName)
	{
		return FileSystem::PathAppend(ResourceDirectoryPath(), folderName);
	}

	[[nodiscard]]
	String DirectoryNameByDirectoryPath(FilePathView directoryPath)
	{
		const bool endsWithSlash = directoryPath.ends_with(U'/') || directoryPath.ends_with(U'\\');
		return FileSystem::FileName(endsWithSlash ? directoryPath.substr(0, directoryPath.size() - 1) : directoryPath);
	}

	[[nodiscard]]
	String EliminateExtension(FilePathView path)
	{
		if (path.empty())
		{
			return String{};
		}
		const auto extension = FileSystem::Extension(path);
		if (extension.empty())
		{
			return String{ path };
		}
		return String{ path.substr(0, path.size() - extension.size() - 1) }; // ドット除去のため-1
	}

	[[nodiscard]]
	String RelativePathFromSongsDir(FilePathView fullPath)
	{
		return FileSystem::RelativePath(fullPath, SongsDirectoryPath());
	}
}
