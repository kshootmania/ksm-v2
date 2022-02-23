#include "i18n.hpp"

Array<String> I18n::GetAvailableLanguageList()
{
	Array<String> langList;
	for (const auto& path : FileSystem::DirectoryContents(kDirectoryPath, Recursive::No))
	{
		if (FileSystem::Extension(path) == U"txt")
		{
			langList.push_back(FileSystem::BaseName(path));
		}
	}
	return langList;
}

void I18n::LoadLanguage(StringView name)
{
	const FilePathView path = U"{}/{}.txt"_fmt(kDirectoryPath, name);
	if (!FileSystem::Exists(path))
	{
		throw Error(U"I18n::LoadLanguage(): Could not find language file '{}'!"_fmt(path));
	}

	// TODO: implement
}
