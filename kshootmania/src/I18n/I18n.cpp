#include "I18n.hpp"
#include "../Common/AssetManagement.hpp"

namespace
{
	I18n::StandardLanguage s_currentLanguage = I18n::StandardLanguage::Unknown;

	// テキストの辞書
	// (カテゴリ名 -> キー名 -> テキスト文字列)
	HashTable<String, HashTable<String, String>> s_dictionary;

	I18n::StandardLanguage ConvertLanguageNameToStandardLanguage(StringView name)
	{
		if (name == U"English")
		{
			return I18n::StandardLanguage::English;
		}
		else if (name == U"Japanese")
		{
			return I18n::StandardLanguage::Japanese;
		}
		else if (name == U"Korean")
		{
			return I18n::StandardLanguage::Korean;
		}
		else if (name == U"Simplified Chinese")
		{
			return I18n::StandardLanguage::SimplifiedChinese;
		}
		else if (name == U"Traditional Chinese")
		{
			return I18n::StandardLanguage::TraditionalChinese;
		}
		else
		{
			return I18n::StandardLanguage::Unknown;
		}
	}

	void LoadDictionaryFromJSON(const FilePath& path)
	{
		s_dictionary.clear();

		const JSON json = JSON::Load(path);
		if (!json)
		{
			Logger << U"[ksm warning] Failed to load language file '{}'"_fmt(path);
			return;
		}

		for (const auto& [category, categoryObj] : json)
		{
			if (!categoryObj.isObject())
			{
				continue;
			}

			HashTable<String, String> categoryDict;
			for (const auto& [key, value] : categoryObj)
			{
				if (value.isString())
				{
					categoryDict[key] = value.getString();
				}
			}
			s_dictionary[category] = std::move(categoryDict);
		}
	}
}

FilePath I18n::GetDirectoryPath()
{
	return FsUtils::GetResourcePath(U"lang");
}

Array<String> I18n::GetAvailableLanguageList()
{
	const FilePath directoryPath = GetDirectoryPath();
	Array<String> langList;
	for (const auto& path : FileSystem::DirectoryContents(directoryPath, Recursive::No))
	{
		if (FileSystem::Extension(path) == U"json")
		{
			langList.push_back(FileSystem::BaseName(path));
		}
	}
	return langList;
}

I18n::StandardLanguage I18n::CurrentLanguage()
{
	return s_currentLanguage;
}

void I18n::LoadLanguage(StringView name, StringView fallback)
{
	const FilePath directoryPath = GetDirectoryPath();
	String path = U"{}/{}.json"_fmt(directoryPath, name);
	s_currentLanguage = ::ConvertLanguageNameToStandardLanguage(name);

	if (!FileSystem::Exists(path))
	{
		Logger << U"[ksm warning] Could not find language file '{}'"_fmt(path);
		path = U"{}/{}.json"_fmt(directoryPath, fallback);
		s_currentLanguage = ::ConvertLanguageNameToStandardLanguage(fallback);
		if (!FileSystem::Exists(path))
		{
			throw Error{ U"I18n::LoadLanguage(): Could not load language '{}' and could not load fallback '{}'."_fmt(name, fallback) };
		}
	}

	LoadDictionaryFromJSON(path);

	AssetManagement::RefreshSystemFontForCurrentLanguage();
}

StringView I18n::GetByCategoryAndKey(const String& category, const String& key)
{
	const auto categoryIt = s_dictionary.find(category);
	if (categoryIt == s_dictionary.end())
	{
		Logger << U"[ksm warning] Category '{}' not found in language dictionary."_fmt(category);
		return U"";
	}

#ifdef __APPLE__
	// macOSではキー名の末尾に"@macOS"が付いたエントリがあれば優先して使用
	const auto platformKeyIt = categoryIt->second.find(key + U"@macOS");
	if (platformKeyIt != categoryIt->second.end())
	{
		return platformKeyIt->second;
	}
#endif

	const auto keyIt = categoryIt->second.find(key);
	if (keyIt == categoryIt->second.end())
	{
		Logger << U"[ksm warning] Key '{}' not found in category '{}'."_fmt(key, category);
		return U"";
	}

	return keyIt->second;
}
