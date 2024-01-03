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
}
