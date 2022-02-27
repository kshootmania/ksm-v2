#include "i18n.hpp"

namespace
{
	std::array<std::array<String, I18n::kKeyIdxMax>, I18n::kCategoryMax> s_i18nDictionary;

	StringView TrimZeroPadding(StringView str)
	{
		const std::size_t length = str.size();
		auto cursor = std::size_t{ 0 };

		while (cursor < length - 1 && str[cursor] == U'0')
		{
			++cursor;
		}

		return str.substr(cursor);
	}
}

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

void I18n::LoadLanguage(StringView name, StringView fallback)
{
	String path = U"{}/{}.txt"_fmt(kDirectoryPath, name);
	if (!FileSystem::Exists(path))
	{
		Print << U"Warning: Could not find language file '{}'!"_fmt(path);
		path = U"{}/{}.txt"_fmt(kDirectoryPath, fallback);
		if (!FileSystem::Exists(path))
		{
			throw Error(U"I18n::LoadLanguage(): Could not load language '{}' and could not load fallback '{}'!"_fmt(name, fallback));
		}
	}

	TextReader reader(path);
	String line;
	int32 lineNumber = 0;
	while (reader.readLine(line))
	{
		lineNumber++;

		constexpr auto kHeaderLength = std::size_t{ 8 }; // Length of "m00-000|"

		if (line.size() >= kHeaderLength && line[0] == U'm' && line[3] == U'-' && line[7] == U'|')
		{
			const StringView categoryIdxStr = TrimZeroPadding(line.substrView(1, 2));
			const StringView keyIdxStr = TrimZeroPadding(line.substrView(4, 3));
			try
			{
				const int32 categoryIdx = Parse<int32>(categoryIdxStr);
				const int32 keyIdx = Parse<int32>(keyIdxStr);

				if (categoryIdx < 0 || kCategoryMax <= categoryIdx || keyIdx < 0 || kKeyIdxMax <= keyIdx)
				{
					Print << U"Warning: Line {} in language file '{}' is ignored! (key: 'm{:0>2}-{:0>3}')"_fmt(lineNumber, path, categoryIdx, keyIdx);
					continue;
				}

				s_i18nDictionary[categoryIdx][keyIdx] = line.substr(kHeaderLength);
			}
			catch (const ParseError&)
			{
				Print << U"Warning: Line {} in language file '{}' is ignored! (Parse error, category:'{}', key:'{}')"_fmt(lineNumber, path, categoryIdxStr, keyIdxStr);
				continue;
			}
		}
	}
}

StringView I18n::Get(Category category, int32 keyIdx)
{
	if (category < 0 || kCategoryMax <= category || keyIdx < 0 || kKeyIdxMax <= keyIdx)
	{
		Print << U"Warning: Failed to read from language dictionary! (key:'m{:0>2}-{:0>3}')"_fmt(category, keyIdx);
		return U"";
	}
	return s_i18nDictionary[category][keyIdx];
}
