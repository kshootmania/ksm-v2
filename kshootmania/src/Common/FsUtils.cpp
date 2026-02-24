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

	FilePath GetFullPathInFolder(SpecialFolder folder, FilePathView relativePath)
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(folder), relativePath);
	}

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

	FilePath ScoreDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"score");
	}

	FilePath SongsDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"songs");
	}

	FilePath SongsDefaultDirectoryPath()
	{
		return FileSystem::PathAppend(ResourceDirectoryPath(), U"songs_default");
	}

	FilePath CoursesDirectoryPath()
	{
		return FileSystem::PathAppend(AppDataDirectoryPath(), U"courses");
	}

	FilePath CourseScoreDirectoryPath()
	{
		return FileSystem::PathAppend(CoursesDirectoryPath(), U"score");
	}

	FilePath GetResourcePath(FilePathView folderName)
	{
		return FileSystem::PathAppend(ResourceDirectoryPath(), folderName);
	}

	String DirectoryNameByDirectoryPath(FilePathView directoryPath)
	{
		const bool endsWithSlash = directoryPath.ends_with(U'/') || directoryPath.ends_with(U'\\');
		return FileSystem::FileName(endsWithSlash ? directoryPath.substr(0, directoryPath.size() - 1) : directoryPath);
	}

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

	String RelativePathFromSongsDir(FilePathView fullPath)
	{
		return FileSystem::RelativePath(fullPath, SongsDirectoryPath());
	}

	Array<FilePath> GetChartFilePathsPreferringKson(FilePathView directoryPath)
	{
		const Array<FilePath> allChartPaths = FileSystem::DirectoryContents(directoryPath, Recursive::No).filter([](FilePathView p)
		{
			return FileSystem::IsFile(p) && HasChartExtension(p);
		});

		Array<FilePath> result;
		HashTable<String, size_t> baseNameToIndex;
		result.reserve(allChartPaths.size());
		baseNameToIndex.reserve(allChartPaths.size());

		for (const auto& path : allChartPaths)
		{
			const String baseName = EliminateExtension(FileSystem::FileName(path));
			const auto [it, inserted] = baseNameToIndex.insert({ baseName, result.size() });

			if (inserted)
			{
				result.emplace_back(path);
			}
			else if (HasKsonExtension(path))
			{
				result[it->second] = path;
			}
		}

		return result;
	}

	bool HasChartExtension(FilePathView filePath)
	{
		const String ext = FileSystem::Extension(filePath);
		return ext == kKSHExtension || ext == kKSONExtension;
	}

	bool HasKsonExtension(FilePathView filePath)
	{
		return FileSystem::Extension(filePath) == kKSONExtension;
	}
}
