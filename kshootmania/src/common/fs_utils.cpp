#include "fs_utils.hpp"

namespace FsUtils
{
	FilePath GetFullPathInFolder(SpecialFolder folder, FilePathView relativePath)
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(folder), relativePath);
	}

	FilePath AppDirectoryPath()
	{
		return FileSystem::ParentPath(FileSystem::ModulePath());
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
}
