#pragma once

struct SelectFolderState
{
	enum FolderType
	{
		kNone = 0,
		kDirectory,
		kAll,
		kFavorite,
	};

	FolderType folderType = kNone;

	String fullPath;
};
